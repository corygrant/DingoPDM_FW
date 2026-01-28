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
    nParkOut = eState == WiperState::Parked ? 1 : 0;
    nInterOut = ((eState == WiperState::IntermittentOn) || (eState == WiperState::IntermittentPause)) ? 1 : 0;
    nWashOut = (eState == WiperState::Wash) ? 1 : 0;
    nSwipeOut = (eState == WiperState::Swipe) ? 1 : 0;
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
            conf->stWiper.bEnabled = Dbc::DecodeInt(rx->data8, 8, 1);
            conf->stWiper.eMode = static_cast<WiperMode>(Dbc::DecodeInt(rx->data8, 9, 2));
            conf->stWiper.bParkStopLevel = Dbc::DecodeInt(rx->data8, 11, 1);
            conf->stWiper.nWashWipeCycles = Dbc::DecodeInt(rx->data8, 12, 4);
            conf->stWiper.nSlowInput = Dbc::DecodeInt(rx->data8, 16, 8);
            conf->stWiper.nFastInput = Dbc::DecodeInt(rx->data8, 24, 8);
            conf->stWiper.nInterInput = Dbc::DecodeInt(rx->data8, 32, 8);
            conf->stWiper.nOnInput = Dbc::DecodeInt(rx->data8, 40, 8);
            conf->stWiper.nParkInput = Dbc::DecodeInt(rx->data8, 48, 8);
            conf->stWiper.nWashInput = Dbc::DecodeInt(rx->data8, 56, 8);
        }

        tx->DLC = 8;
        tx->IDE = CAN_IDE_STD;

        for (int i = 0; i < 8; i++) tx->data8[i] = 0;
        tx->data8[0] = static_cast<uint8_t>(MsgCmd::Wiper) + 128;
        Dbc::EncodeInt(tx->data8, conf->stWiper.bEnabled, 8, 1);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eMode), 9, 2);
        Dbc::EncodeInt(tx->data8, conf->stWiper.bParkStopLevel, 11, 1);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nWashWipeCycles, 12, 4);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nSlowInput, 16, 8);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nFastInput, 24, 8);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nInterInput, 32, 8);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nOnInput, 40, 8);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nParkInput, 48, 8);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nWashInput, 56, 8);

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
            conf->stWiper.nSwipeInput = Dbc::DecodeInt(rx->data8, 8, 8);
            conf->stWiper.nSpeedInput = Dbc::DecodeInt(rx->data8, 16, 8);
            conf->stWiper.eSpeedMap[0] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 24, 4));
            conf->stWiper.eSpeedMap[1] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 28, 4));
            conf->stWiper.eSpeedMap[2] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 32, 4));
            conf->stWiper.eSpeedMap[3] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 36, 4));
            conf->stWiper.eSpeedMap[4] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 40, 4));
            conf->stWiper.eSpeedMap[5] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 44, 4));
            conf->stWiper.eSpeedMap[6] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 48, 4));
            conf->stWiper.eSpeedMap[7] = static_cast<WiperSpeed>(Dbc::DecodeInt(rx->data8, 52, 4));
        }

        tx->DLC = 7;
        tx->IDE = CAN_IDE_STD;

        for (int i = 0; i < 8; i++) tx->data8[i] = 0;
        tx->data8[0] = static_cast<uint8_t>(MsgCmd::WiperSpeed) + 128;
        Dbc::EncodeInt(tx->data8, conf->stWiper.nSwipeInput, 8, 8);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nSpeedInput, 16, 8);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[0]), 24, 4);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[1]), 28, 4);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[2]), 32, 4);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[3]), 36, 4);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[4]), 40, 4);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[5]), 44, 4);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[6]), 48, 4);
        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stWiper.eSpeedMap[7]), 52, 4);

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
            conf->stWiper.nIntermitTime[0] = Dbc::DecodeInt(rx->data8, 8, 8, 100.0f);
            conf->stWiper.nIntermitTime[1] = Dbc::DecodeInt(rx->data8, 16, 8, 100.0f);
            conf->stWiper.nIntermitTime[2] = Dbc::DecodeInt(rx->data8, 24, 8, 100.0f);
            conf->stWiper.nIntermitTime[3] = Dbc::DecodeInt(rx->data8, 32, 8, 100.0f);
            conf->stWiper.nIntermitTime[4] = Dbc::DecodeInt(rx->data8, 40, 8, 100.0f);
            conf->stWiper.nIntermitTime[5] = Dbc::DecodeInt(rx->data8, 48, 8, 100.0f);
        }

        tx->DLC = 7;
        tx->IDE = CAN_IDE_STD;

        for (int i = 0; i < 8; i++) tx->data8[i] = 0;
        tx->data8[0] = static_cast<uint8_t>(MsgCmd::WiperDelays) + 128;
        Dbc::EncodeInt(tx->data8, conf->stWiper.nIntermitTime[0], 8, 8, 100.0f);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nIntermitTime[1], 16, 8, 100.0f);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nIntermitTime[2], 24, 8, 100.0f);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nIntermitTime[3], 32, 8, 100.0f);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nIntermitTime[4], 40, 8, 100.0f);
        Dbc::EncodeInt(tx->data8, conf->stWiper.nIntermitTime[5], 48, 8, 100.0f);

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

void Wiper::SetDefaultConfig(Config_Wiper *config)
{
    config->bEnabled = false;
    config->eMode = WiperMode::DigIn;
    config->nSlowInput = 0;
    config->nFastInput = 0;
    config->nInterInput = 0;
    config->nOnInput = 0;
    config->nSpeedInput = 0;
    config->nParkInput = 0;
    config->bParkStopLevel = false;
    config->nSwipeInput = 0;
    config->nWashInput = 0;
    config->nWashWipeCycles = 0;
    config->eSpeedMap[0] = WiperSpeed::Intermittent1;
    config->eSpeedMap[1] = WiperSpeed::Intermittent2;
    config->eSpeedMap[2] = WiperSpeed::Intermittent3;
    config->eSpeedMap[3] = WiperSpeed::Intermittent4;  
    config->eSpeedMap[4] = WiperSpeed::Intermittent5;
    config->eSpeedMap[5] = WiperSpeed::Intermittent6;
    config->eSpeedMap[6] = WiperSpeed::Slow;
    config->eSpeedMap[7] = WiperSpeed::Fast;
    config->nIntermitTime[0] = 1000;
    config->nIntermitTime[1] = 2000;
    config->nIntermitTime[2] = 3000;
    config->nIntermitTime[3] = 4000;
    config->nIntermitTime[4] = 5000;
    config->nIntermitTime[5] = 6000;
}