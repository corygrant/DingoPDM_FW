#include "wiper.h"

void Wiper::Update(uint32_t nTimeNow)
{
    if (!pConfig->bEnabled)
        return;

    switch (eState)
    {
    case WiperState::Parked:
        Parked();
        break;

    case WiperState::Parking:
        Parking();
        break;

    case WiperState::Slow:
        Slow();
        break;

    case WiperState::Fast:
        Fast();
        break;

    case WiperState::IntermittentOn:
        InterOn(nTimeNow);
        break;

    case WiperState::IntermittentPause:
        InterPause(nTimeNow);
        break;

    case WiperState::Wash:
        Wash();
        break;

    case WiperState::Swipe:
        Swipe();
        break;
    }

    if (*pWashInput)
    {
        eStatePreWash = eState;
        eState = WiperState::Wash;
    }

    if (*pSwipeInput)
        eState = WiperState::Swipe;


    eState = eNextState;
}

void Wiper::SetMotorSpeed(MotorSpeed speed)
{
    switch (speed)
    {
    case MotorSpeed::Off:
        nSlowOut = 0;
        nFastOut = 0;
        break;

    case MotorSpeed::Slow:
        nSlowOut = 1;
        nFastOut = 0;
        break;

    case MotorSpeed::Fast:
        nSlowOut = 1;
        nFastOut = 1;
        break;
    }
}

void Wiper::ReqState(WiperState state)
{
    eNextState = state;
}

void Wiper::Parked()
{
    SetMotorSpeed(MotorSpeed::Off);
}

void Wiper::Parking()
{
    // Park detected - stop motor
    // Park high or low depending on stop level
    if ((!(*pParkSw) && !pConfig->bParkStopLevel) ||
        (*pParkSw && pConfig->bParkStopLevel))
    {
        SetMotorSpeed(MotorSpeed::Off);
        eState = WiperState::Parked;
    }
}

void Wiper::Slow()
{
    SetMotorSpeed(MotorSpeed::Slow);
    ModeUpdate();
}

void Wiper::Fast()
{
    SetMotorSpeed(MotorSpeed::Fast);
    ModeUpdate();
}

void Wiper::InterOn(uint32_t nTimeNow)
{
    SetMotorSpeed(MotorSpeed::Slow);
    ModeUpdate();

    // Park detected
    // Stop motor
    // Save time - pause for set time
    if (!(*pParkSw))
    {
        nInterPauseStartTime = nTimeNow;
        eState = WiperState::IntermittentPause;
    }
}

void Wiper::InterPause(uint32_t nTimeNow)
{
    SetMotorSpeed(MotorSpeed::Off);
    ModeUpdate();

    // Pause for inter delay
    if ((nTimeNow - nInterPauseStartTime) > nInterDelay)
    {
        SetMotorSpeed(MotorSpeed::Slow);
        eState = WiperState::IntermittentOn;
    }
}

void Wiper::Wash()
{
    if (eStatePreWash == WiperState::Fast)
        SetMotorSpeed(MotorSpeed::Fast);
    else
        SetMotorSpeed(MotorSpeed::Slow);

    if (*pWashInput)
    {
        nWashWipeCount = 0;
    }
    else
    {
        if (!(*pParkSw) && (*pParkSw != nLastParkSw))
            nWashWipeCount++;

        if (nWashWipeCount >= pConfig->nWashWipeCycles)
        {
            if (eStatePreWash == WiperState::Parked)
                ReqState(WiperState::Parking);
            else if (eStatePreWash == WiperState::IntermittentPause)
            {
                ReqState(WiperState::IntermittentOn);
            }
            else
            {
                ReqState(eStatePreWash);
            }
        }
    }
    nLastParkSw = *pParkSw;
}

void Wiper::Swipe()
{
    eLastState = eState;

    SetMotorSpeed(MotorSpeed::Fast);

    // Park switch high
    // Moved past park position
    if (*pParkSw)
    {
        eState = WiperState::Parking;
    }
}

void Wiper::UpdateInter()
{
    if (eSelectedSpeed >= WiperSpeed::Intermittent1 && eSelectedSpeed <= WiperSpeed::Intermittent6)
    {
        nInterDelay = pConfig->nIntermitTime[static_cast<int>(eSelectedSpeed) - 3];
    }
}

void Wiper::ModeUpdate()
{
    switch (pConfig->eMode)
    {
    case WiperMode::DigIn:
        DigInUpdate();
        break;

    case WiperMode::IntIn:
        IntInUpdate();
        break;

    case WiperMode::MixIn:
        MixInUpdate();
        break;
    }
}

void Wiper::DigInUpdate()
{
    if (*pInterInput)
    {
        //No speed input, use first delay
        nInterDelay = pConfig->nIntermitTime[0];
        ReqState(WiperState::IntermittentOn);
    }

    if (*pSlowInput)
        ReqState(WiperState::Slow);

    if (*pFastInput)
        ReqState(WiperState::Fast);

    if (eState == WiperState::Fast)
    {
        if (!(*pFastInput))
            ReqState(WiperState::Parking);
    }

    if (eState == WiperState::Slow)
    {
        if (!(*pSlowInput))
            ReqState(WiperState::Parking);
    }

    if ((eState == WiperState::IntermittentOn) ||
        (eState == WiperState::IntermittentPause))
    {
        if (!(*pInterInput))
            ReqState(WiperState::Parking);
    }
}
void Wiper::IntInUpdate()
{
    eSelectedSpeed = pConfig->eSpeedMap[*pSpeedInput];
    
    switch (eSelectedSpeed)
    {
    case WiperSpeed::Park:
        ReqState(WiperState::Parking);
        break;

    case WiperSpeed::Slow:
        ReqState(WiperState::Slow);
        break;

    case WiperSpeed::Fast:
        ReqState(WiperState::Fast);
        break;

    case WiperSpeed::Intermittent1... WiperSpeed::Intermittent6:
        UpdateInter();
        ReqState(WiperState::IntermittentOn);
        break;
    }
}

void Wiper::MixInUpdate()
{
    eSelectedSpeed = pConfig->eSpeedMap[*pSpeedInput];

    // Wipers turned off - park
    if (!(*pOnSw))
        ReqState(WiperState::Parking);

    // Speed changed
    switch (eSelectedSpeed)
    {
    case WiperSpeed::Park:
        //Not active in MixIn mode
        break;

    case WiperSpeed::Slow:
        ReqState(WiperState::Slow);
        break;

    case WiperSpeed::Fast:
        ReqState(WiperState::Fast);
        break;

    case WiperSpeed::Intermittent1... WiperSpeed::Intermittent6:
        UpdateInter();
        ReqState(WiperState::IntermittentOn);
        break;
    }
}