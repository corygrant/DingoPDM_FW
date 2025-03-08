#include "wiper.h"

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
}

void Wiper::Parking()
{
    // Turn motor on to move back to park position
    if (GetMotorSpeed() == MotorSpeed::Off)
        SetMotorSpeed(MotorSpeed::Slow);

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
    if (!((SYS_TIME - nMotorOnTime) > 100))
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

MotorSpeed Wiper::GetMotorSpeed()
{
    if (nSlowOut && nFastOut)
        return MotorSpeed::Fast;
    else if (nSlowOut)
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

MsgCmdResult WiperMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
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
                       ((static_cast<uint8_t>(conf->stWiper.eMode) & 0x03) << 1) +
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

MsgCmdResult WiperSpeedMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
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

MsgCmdResult WiperDelaysMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
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

MsgCmdResult Wiper::ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
{
    MsgCmd cmd = static_cast<MsgCmd>(rx->data8[0]);

    if (cmd == MsgCmd::Wiper)
        return WiperMsg(conf, rx, tx);
    else if (cmd == MsgCmd::WiperSpeed)
        return WiperSpeedMsg(conf, rx, tx);
    else if (cmd == MsgCmd::WiperDelays)
        return WiperDelaysMsg(conf, rx, tx);

    return MsgCmdResult::Invalid;
}