#include "wiper.h"
#include "dbc.h"

void Wiper::Update()
{
    if (!pConfig->bEnabled)
    {
        SetMotorSpeed(MotorSpeed::Off);
        return;
    }

    UpdateInter();

    CheckWash();
    CheckSwipe();

    switch (eState)
    {
    case WiperState::Parked:
        eLastState = eState;
        SetMotorSpeed(MotorSpeed::Off);
        break;

    case WiperState::Parking:
        eLastState = eState;
        // Keep last speed
        Parking();
        break;

    case WiperState::Slow:
        eLastState = eState;
        SetMotorSpeed(MotorSpeed::Slow);
        break;

    case WiperState::Fast:
        eLastState = eState;
        SetMotorSpeed(MotorSpeed::Fast);
        break;

    case WiperState::IntermittentOn:
        eLastState = eState;
        SetMotorSpeed(MotorSpeed::Slow);
        InterOn();
        break;

    case WiperState::IntermittentPause:
        eLastState = eState;
        SetMotorSpeed(MotorSpeed::Off);
        InterPause();
        break;

    case WiperState::Wash:
        eLastState = eState;
        // Set speed to prewash speed in Wash()
        Wash();
        break;

    case WiperState::Swipe:
        eLastState = eState;
        SetMotorSpeed(MotorSpeed::Fast);
        Swipe();
        break;
    }

    pMode->CheckInputs();

    //Set var map values
    //Fast/Slow set in SetMotorSpeed()
    fParkOut = eState == WiperState::Parked ? 1 : 0;
    fInterOut = ((eState == WiperState::IntermittentOn) || (eState == WiperState::IntermittentPause)) ? 1 : 0;
    fWashOut = (eState == WiperState::Wash) ? 1 : 0;
    fSwipeOut = (eState == WiperState::Swipe) ? 1 : 0;
}

void Wiper::Parking()
{
    // Park detected - stop motor
    if (GetParkSw())
    {
        SetMotorSpeed(MotorSpeed::Off);
        eState = WiperState::Parked;
        eSelectedSpeed = WiperSpeed::Park;
    }
}

void Wiper::InterOn()
{
    //Delay checking switch to allow motor to spin off of switch
    if (!((SYS_TIME - nMotorOnTime) > 1000))
        return;

    // Park detected
    // Stop motor
    // Save time - pause for set time
    if (GetParkSw())
    {
        nInterPauseStartTime = SYS_TIME;
        eState = WiperState::IntermittentPause;
    }
}

void Wiper::InterPause()
{
    // Pause for inter delay
    if ((SYS_TIME - nInterPauseStartTime) > nInterDelay)
    {
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
        //Delay checking switch to allow motor to spin off of switch
        if (!((SYS_TIME - nMotorOnTime) > 100))
            return;
        
        if (GetParkSw() && (GetParkSw() != nLastParkSw))
            nWashWipeCount++;

        if (nWashWipeCount >= pConfig->nWashWipeCycles)
        {
            if (eStatePreWash == WiperState::Parked)
                eState = WiperState::Parking;
            else if (eStatePreWash == WiperState::IntermittentPause)
            {
                eState = WiperState::IntermittentOn;
            }
            else
            {
                eState = eStatePreWash;
            }
        }
    }
    nLastParkSw = GetParkSw();
}

void Wiper::Swipe()
{
    //Delay checking switch to allow motor to spin off of switch
    if (!((SYS_TIME - nMotorOnTime) > 100))
        return;

    // Park switch high
    // Moved past park position
    if (GetParkSw())
    {
        eState = WiperState::Parking;
    }
}

void Wiper::SetMotorSpeed(MotorSpeed speed)
{
    if((speed != eLastMotorSpeed) && (speed != MotorSpeed::Off))
        nMotorOnTime = SYS_TIME;
    
    eLastMotorSpeed = speed;
        
    switch (speed)
    {
    case MotorSpeed::Off:
        fSlowOut = 0;
        fFastOut = 0;
        break;

    case MotorSpeed::Slow:
        fSlowOut = 1;
        fFastOut = 0;
        break;

    case MotorSpeed::Fast:
        fSlowOut = 1;
        fFastOut = 1;
        break;
    }
    
}

MotorSpeed Wiper::GetMotorSpeed()
{
    if (fSlowOut && fFastOut)
        return MotorSpeed::Fast;
    else if (fSlowOut)
        return MotorSpeed::Slow;
    else
        return MotorSpeed::Off;
}

void Wiper::UpdateInter()
{
    // Map intermittent speed to delay
    if (eSelectedSpeed >= WiperSpeed::Intermittent1 && eSelectedSpeed <= WiperSpeed::Intermittent6)
    {
        nInterDelay = pConfig->nIntermitTime[static_cast<int>(eSelectedSpeed) - 3];
    }
}

void Wiper::CheckWash()
{
    if (GetWashInput() && (eState != WiperState::Wash))
    {
        eStatePreWash = eState;
        eState = WiperState::Wash;
    }
}

void Wiper::CheckSwipe()
{
    if (GetSwipeInput() && (eState == WiperState::Parked))
    {
        eState = WiperState::Swipe;
    }
}

bool Wiper::GetParkSw()
{
    return (!(*pParkSw) && !pConfig->bParkStopLevel) ||
           (*pParkSw && pConfig->bParkStopLevel);
}