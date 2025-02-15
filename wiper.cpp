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

MsgCmdResult WiperMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set wiper settings
    // DLC 1 = Get wiper settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 1))
    {
        if (rx->DLC == 8)
        {
            conf->stWiper.bEnabled = (rx->data8[1] & 0x01);
            conf->stWiper.eMode = static_cast<WiperMode>((rx->data8[1] & 0x06) >> 1);
            conf->stWiper.bParkStopLevel = (rx->data8[1] & 0x08) >> 3;
            conf->stWiper.nWashWipeCycles = (rx->data8[1] & 0xF0) >> 4;
            conf->stWiper.nSlowInput = rx->data8[2];
            conf->stWiper.nFastInput = rx->data8[3];
            conf->stWiper.nInterInput = rx->data8[4];
            conf->stWiper.nOnInput = rx->data8[5];
            conf->stWiper.nParkInput = rx->data8[6];
            conf->stWiper.nWashInput = rx->data8[7];
        }

        tx->DLC = 8;
        tx->IDE = CAN_IDE_STD;

        tx->data8[0] = static_cast<uint8_t>(MsgCmd::Wiper) + 128;
        tx->data8[1] = ((conf->stWiper.nWashWipeCycles & 0x0F) << 4) +
                      ((conf->stWiper.bParkStopLevel & 0x01) << 3) +
                      ((static_cast<uint8_t>(conf->stWiper.eMode) & 0x03) << 2) +
                      (conf->stWiper.bEnabled & 0x01);
        tx->data8[2] = conf->stWiper.nSlowInput;
        tx->data8[3] = conf->stWiper.nFastInput;
        tx->data8[4] = conf->stWiper.nInterInput;
        tx->data8[5] = conf->stWiper.nOnInput;
        tx->data8[6] = conf->stWiper.nParkInput;
        tx->data8[7] = conf->stWiper.nWashInput;

        if (rx->DLC == 8)
            return MsgCmdResult::Write;
        else
            return MsgCmdResult::Request;
    }
    return MsgCmdResult::Invalid;
}

MsgCmdResult WiperSpeedMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 7 = Set wiper speed settings
    // DLC 1 = Get wiper speed settings

    if ((rx->DLC == 7) ||
        (rx->DLC == 1))
    {
        if (rx->DLC == 7)
        {
            conf->stWiper.nSwipeInput = rx->data8[1];
            conf->stWiper.nSpeedInput = rx->data8[2];
            conf->stWiper.eSpeedMap[0] = static_cast<WiperSpeed>((rx->data8[3] & 0x0F));
            conf->stWiper.eSpeedMap[1] = static_cast<WiperSpeed>((rx->data8[3] & 0xF0) >> 4);
            conf->stWiper.eSpeedMap[2] = static_cast<WiperSpeed>((rx->data8[4] & 0x0F));
            conf->stWiper.eSpeedMap[3] = static_cast<WiperSpeed>((rx->data8[4] & 0xF0) >> 4);
            conf->stWiper.eSpeedMap[4] = static_cast<WiperSpeed>((rx->data8[5] & 0x0F));
            conf->stWiper.eSpeedMap[5] = static_cast<WiperSpeed>((rx->data8[5] & 0xF0) >> 4);
            conf->stWiper.eSpeedMap[6] = static_cast<WiperSpeed>((rx->data8[6] & 0x0F));
            conf->stWiper.eSpeedMap[7] = static_cast<WiperSpeed>((rx->data8[6] & 0xF0) >> 4);
        }

        tx->DLC = 7;
        tx->IDE = CAN_IDE_STD;

        tx->data8[0] = static_cast<uint8_t>(MsgCmd::WiperSpeed) + 128;
        tx->data8[1] = conf->stWiper.nSwipeInput;
        tx->data8[2] = conf->stWiper.nSpeedInput;
        tx->data8[3] = ((static_cast<uint8_t>(conf->stWiper.eSpeedMap[1]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(conf->stWiper.eSpeedMap[0]) & 0x0F);
        tx->data8[4] = ((static_cast<uint8_t>(conf->stWiper.eSpeedMap[3]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(conf->stWiper.eSpeedMap[2]) & 0x0F);
        tx->data8[5] = ((static_cast<uint8_t>(conf->stWiper.eSpeedMap[5]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(conf->stWiper.eSpeedMap[4]) & 0x0F);
        tx->data8[6] = ((static_cast<uint8_t>(conf->stWiper.eSpeedMap[7]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(conf->stWiper.eSpeedMap[6]) & 0x0F);
        tx->data8[7] = 0;

        if (rx->DLC == 7)
            return MsgCmdResult::Write;
        else
            return MsgCmdResult::Request;
    }
    return MsgCmdResult::Invalid;
}

MsgCmdResult WiperDelaysMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 7 = Set wiper delay settings
    // DLC 1 = Get wiper delay settings

    if ((rx->DLC == 7) ||
        (rx->DLC == 1))
    {
        if (rx->DLC == 7)
        {
            conf->stWiper.nIntermitTime[0] = rx->data8[1] * 100;
            conf->stWiper.nIntermitTime[1] = rx->data8[2] * 100;
            conf->stWiper.nIntermitTime[2] = rx->data8[3] * 100;
            conf->stWiper.nIntermitTime[3] = rx->data8[4] * 100;
            conf->stWiper.nIntermitTime[4] = rx->data8[5] * 100;
            conf->stWiper.nIntermitTime[5] = rx->data8[6] * 100;
        }

        tx->DLC = 7;
        tx->IDE = CAN_IDE_STD;

        tx->data8[0] = static_cast<uint8_t>(MsgCmd::WiperDelays) + 128;
        tx->data8[1] = conf->stWiper.nIntermitTime[0] / 100;
        tx->data8[2] = conf->stWiper.nIntermitTime[1] / 100;
        tx->data8[3] = conf->stWiper.nIntermitTime[2] / 100;
        tx->data8[4] = conf->stWiper.nIntermitTime[3] / 100;
        tx->data8[5] = conf->stWiper.nIntermitTime[4] / 100;
        tx->data8[6] = conf->stWiper.nIntermitTime[5] / 100;
        tx->data8[7] = 0;

        if (rx->DLC == 7)
            return MsgCmdResult::Write;
        else
            return MsgCmdResult::Request;
    }
    return MsgCmdResult::Invalid;
}

MsgCmdResult Wiper::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    MsgCmd cmd = static_cast<MsgCmd>(rx->data8[0]);

    if(cmd == MsgCmd::Wiper)
        return WiperMsg(conf, rx, tx);
    else if(cmd == MsgCmd::WiperSpeed)
        return WiperSpeedMsg(conf, rx, tx);
    else if(cmd == MsgCmd::WiperDelays)
        return WiperDelaysMsg(conf, rx, tx);

    return MsgCmdResult::Invalid;

}