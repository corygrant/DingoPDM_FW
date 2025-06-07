#include "keypad.h"
#include "dingopdm_config.h"

#define KEYPAD_NUM_TX_MSGS 4

static THD_WORKING_AREA(waKeypadThread, 128);
void KeypadThread(void *arg)
{
    Keypad *keypad = static_cast<Keypad *>(arg);

    chRegSetThreadName("Keypad");

    CANTxFrame msg = keypad->GetStartMsg();
    PostTxFrame(&msg);
    chThdSleepMilliseconds(KEYPAD_TX_MSG_SPLIT);

    while (1)
    {
        for(uint8_t i = 0; i < KEYPAD_NUM_TX_MSGS; i++)
        {
            msg = keypad->GetTxMsg(i);
            PostTxFrame(&msg);
            chThdSleepMilliseconds(KEYPAD_TX_MSG_SPLIT);
        }

        chThdSleepMilliseconds(KEYPAD_TX_MSG_DELAY);
    }
}

msg_t Keypad::Init()
{
    if(pConfig->bEnabled == false)
        return MSG_RESET;
        
    switch(pConfig->eModel)
    {
        case KeypadModel:: Blink2Key:
            nNumButtons = 2;
            nNumDials = 0;
            break;
        case KeypadModel::Blink4Key:
            nNumButtons = 4;
            nNumDials = 0;
            break;
        case KeypadModel::Blink5Key:
            nNumButtons = 5;
            nNumDials = 0;
            break;
        case KeypadModel::Blink6Key:
            nNumButtons = 6;
            nNumDials = 0;
            break;
        case KeypadModel::Blink8Key:
            nNumButtons = 8;
            nNumDials = 0;
            break;
        case KeypadModel::Blink10Key:
            nNumButtons = 10;
            nNumDials = 0;
            break;
        case KeypadModel::Blink12Key:
            nNumButtons = 12;
            nNumDials = 0;
            break;
        case KeypadModel::Blink15Key:
            nNumButtons = 15;
            nNumDials = 0;
            break;
        case KeypadModel::Blink13Key2Dial:
            nNumButtons = 13;
            nNumDials = 2;
            break;
        case KeypadModel::BlinkRacepad:
            nNumButtons = 12;
            nNumDials = 4;
            break;
        case KeypadModel::Grayhill6Key:
            nNumButtons = 6;
            nNumDials = 0;
            break;
        case KeypadModel::Grayhill8Key:
            nNumButtons = 8;
            nNumDials = 0;
            break;
        case KeypadModel::Grayhill15Key:
            nNumButtons = 15;
            nNumDials = 0;
            break;
        case KeypadModel::Grayhill20Key:  
            nNumButtons = 20;
            nNumDials = 0;
            break;
        default:
            nNumButtons = 0;
            nNumDials = 0;
            return MSG_RESET;
    }

    chThdCreateStatic(waKeypadThread, sizeof(waKeypadThread), NORMALPRIO, KeypadThread, this);
    return MSG_OK;
}

void Keypad::CheckTimeout()
{
    if (!pConfig->bEnabled)
        return;

    if (!pConfig->bTimeoutEnabled)
        return;

    if (SYS_TIME - nLastRxTime > pConfig->nTimeout)
    {
        for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        {
            nVal[i] = 0;
        }
    }
}

bool Keypad::CheckMsg(CANRxFrame frame)
{
    if (!pConfig->bEnabled)
        return false;

    if (frame.SID != pConfig->nNodeId + 0x180)
        return false;

    nLastRxTime = SYS_TIME;

    nVal[0] = button[0].Update(frame.data8[0] & 0x01);
    nVal[1] = button[1].Update((frame.data8[0] & 0x02) >> 1);
    nVal[2] = button[2].Update((frame.data8[0] & 0x04) >> 2);
    nVal[3] = button[3].Update((frame.data8[0] & 0x08) >> 3);
    nVal[4] = button[4].Update((frame.data8[0] & 0x10) >> 4);
    nVal[5] = button[5].Update((frame.data8[0] & 0x20) >> 5);
    nVal[6] = button[6].Update((frame.data8[0] & 0x40) >> 6);
    nVal[7] = button[7].Update((frame.data8[0] & 0x80) >> 7);
    nVal[8] = button[8].Update((frame.data8[1] & 0x01));
    nVal[9] = button[9].Update((frame.data8[1] & 0x02) >> 1);
    nVal[10] = button[10].Update((frame.data8[1] & 0x04) >> 2);
    nVal[11] = button[11].Update((frame.data8[1] & 0x08) >> 3);
    nVal[12] = button[12].Update((frame.data8[1] & 0x10) >> 4);
    nVal[13] = button[13].Update((frame.data8[1] & 0x20) >> 5);
    nVal[14] = button[14].Update((frame.data8[1] & 0x40) >> 6);
    nVal[15] = 0;
    nVal[16] = 0;
    nVal[17] = 0;
    nVal[18] = 0;
    nVal[19] = 0;


    //nNodeId + 0x280
        //If not racepad:
        //dial[0].Update(frame.data64[0]);
        //If racepad:
        //dial[0].Update(frame.data64[0]);
        //dial[1].Update(frame.data64[0]);

    //nNodeId + 0x380
        //If not racepad:
        //dial[1].Update(frame.data64[0]);
        //If racepad:
        //dial[2].Update(frame.data64[0]);
        //dial[3].Update(frame.data64[0]);

    nDialVal[0] = dial[0].GetTicks();
    nDialVal[1] = dial[1].GetTicks();
    nDialVal[2] = dial[2].GetTicks();
    nDialVal[3] = dial[3].GetTicks();

    return true;
}

uint64_t Keypad::BuildLedMsg(bool bBlink)
{
    uint64_t nMsg = 0;
    //Blink Marine PKP-2600SI 12 button keypad
    //Has different LED mapping than other keypads
    //Stacked
    if (pConfig->eModel == KeypadModel::Blink12Key)
    {
        uint8_t nIndex = 0;
        for (uint8_t i = 0; i < nNumButtons; i++)
        {
            nMsg |= (ColorToRed(bBlink ? button[i].eLedBlinkColor : button[i].eLedOnColor) << nIndex++);
        }
        for (uint8_t i = 0; i < nNumButtons; i++)
        {
            nMsg |= (ColorToGreen(bBlink ? button[i].eLedBlinkColor : button[i].eLedOnColor) << nIndex++);
        }
        for (uint8_t i = 0; i < nNumButtons; i++)
        {
            nMsg |= (ColorToBlue(bBlink ? button[i].eLedBlinkColor : button[i].eLedOnColor) << nIndex++);
        }

        return nMsg;
    }
     
    //All other Blink keypads have padding
    //Padded
    uint8_t nBytesPerColor = (nNumButtons + 7) / 8;  // Ceiling
    uint8_t nByteIndex = 0;
    uint8_t nBitIndex = 0;
    uint8_t nBitPosition = 0;

    for (uint8_t i = 0; i < nNumButtons; i++) {
        nByteIndex = i / 8;
        nBitIndex = i % 8;
        nBitPosition = (nByteIndex * 8) + nBitIndex;
        nMsg |= (ColorToRed(bBlink ? button[i].eLedBlinkColor : button[i].eLedOnColor) << nBitPosition);
    }
    
    for (uint8_t i = 0; i < nNumButtons; i++) {
        nByteIndex = nBytesPerColor + (i / 8);
        nBitIndex = i % 8;
        nBitPosition = (nByteIndex * 8) + nBitIndex;
        nMsg |= (ColorToGreen(bBlink ? button[i].eLedBlinkColor : button[i].eLedOnColor) << nBitPosition);
    }
    
    for (uint8_t i = 0; i < nNumButtons; i++) {
        nByteIndex = 2 * nBytesPerColor + (i / 8);
        nBitIndex = i % 8;
        nBitPosition = (nByteIndex * 8) + nBitIndex;
        nMsg |= (ColorToBlue(bBlink ? button[i].eLedBlinkColor : button[i].eLedOnColor) << nBitPosition);
    }

    return nMsg;
}

CANTxFrame Keypad::LedOnMsg()
{
    for (uint8_t i = 0; i < nNumButtons; i++)
        button[i].UpdateLed();

    CANTxFrame msg;
    msg.SID = pConfig->nNodeId + 0x200;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data64[0] = BuildLedMsg(false);
    
    return msg;
}

CANTxFrame Keypad::LedBlinkMsg()
{
    for (uint8_t i = 0; i < nNumButtons; i++)
        button[i].UpdateLed();

    CANTxFrame msg;
    msg.SID = pConfig->nNodeId + 0x300;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data64[0] = BuildLedMsg(true);
    
    return msg;
}

CANTxFrame Keypad::LedBrightnessMsg()
{
    CANTxFrame msg;
    msg.SID = pConfig->nNodeId + 0x400;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data8[0] = *pDimmingInput ? pConfig->nDimButtonBrightness : pConfig->nButtonBrightness;
    msg.data8[1] = 0x00;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}
CANTxFrame Keypad::BacklightMsg()
{
    CANTxFrame msg;
    msg.SID = pConfig->nNodeId + 0x500;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data8[0] = *pDimmingInput ? pConfig->nDimBacklightBrightness : pConfig->nBacklightBrightness;
    msg.data8[1] = pConfig->nBacklightColor;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}

bool Keypad::ColorToRed(BlinkMarineButtonColor eColor)
{
    return (static_cast<uint8_t>(eColor) & 0x01);
}

bool Keypad::ColorToGreen(BlinkMarineButtonColor eColor)
{
    return ((static_cast<uint8_t>(eColor) & 0x02) >> 1);
}

bool Keypad::ColorToBlue(BlinkMarineButtonColor eColor)
{
    return ((static_cast<uint8_t>(eColor) & 0x04) >> 2);
}

CANTxFrame Keypad::GetTxMsg(uint8_t nIndex)
{
    CANTxFrame msg;

    switch (nIndex)
    {
    case 0:
        msg = LedOnMsg();
        break;
    case 1:
        msg = LedBlinkMsg();
        break;
    case 2:
        msg = LedBrightnessMsg();
        break;
    case 3:
        msg = BacklightMsg();
        break;
    default:
        break;
    }

    return msg;
}

CANTxFrame Keypad::GetStartMsg()
{
    CANTxFrame msg;
    msg.SID = 0x00;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data8[0] = 0x01;
    msg.data8[1] = pConfig->nNodeId;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}

MsgCmdResult KeypadMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 6 = Set keypad settings
    // DLC 2 = Get keypad settings

    if ((rx->DLC == 6) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 6)
            {
                conf->stKeypad[nIndex].bEnabled = (rx->data8[2] & 0x01);
                
                conf->stKeypad[nIndex].nNodeId = (rx->data8[3] & 0x7F);
                conf->stKeypad[nIndex].bTimeoutEnabled = (rx->data8[3] & 0x80) >> 7;
        
                conf->stKeypad[nIndex].nTimeout = (rx->data8[4] * 100);

                conf->stKeypad[nIndex].eModel = static_cast<KeypadModel>(rx->data8[5]);
            }

            tx->DLC = 6;
            tx->IDE = CAN_IDE_STD;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::Keypad) + 128;
            tx->data8[1] = nIndex;
            tx->data8[2] =  (conf->stKeypad[nIndex].bEnabled & 0x01);
            tx->data8[3] =  (conf->stKeypad[nIndex].nNodeId & 0x7F) + 
                            ((conf->stKeypad[nIndex].bTimeoutEnabled & 0x01) << 7);
            tx->data8[4] = (uint8_t)(conf->stKeypad[nIndex].nTimeout / 100);
            tx->data8[5] = static_cast<uint8_t>(conf->stKeypad[nIndex].eModel);

            if(rx->DLC == 6)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

MsgCmdResult KeypadLedMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set keypad LED settings
    // DLC 2 = Get keypad LED settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 8)
            {
                conf->stKeypad[nIndex].nBacklightBrightness = rx->data8[2];
                conf->stKeypad[nIndex].nBacklightColor = rx->data8[3];
                conf->stKeypad[nIndex].nDimBacklightBrightness = rx->data8[4];
                conf->stKeypad[nIndex].nDimmingVar = rx->data8[5];
                conf->stKeypad[nIndex].nButtonBrightness = rx->data8[6];
                conf->stKeypad[nIndex].nDimButtonBrightness = rx->data8[7];
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::KeypadLed) + 128;
            tx->data8[1] = nIndex;
            tx->data8[2] = conf->stKeypad[nIndex].nBacklightBrightness;
            tx->data8[3] = conf->stKeypad[nIndex].nBacklightColor;
            tx->data8[4] = conf->stKeypad[nIndex].nDimBacklightBrightness;
            tx->data8[5] = conf->stKeypad[nIndex].nDimmingVar;
            tx->data8[6] = conf->stKeypad[nIndex].nButtonBrightness;
            tx->data8[7] = conf->stKeypad[nIndex].nDimButtonBrightness;

            if(rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

MsgCmdResult Keypad::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    MsgCmd cmd = static_cast<MsgCmd>(rx->data8[0]);

    if(cmd == MsgCmd::Keypad)
        return KeypadMsg(conf, rx, tx);
    else if(cmd == MsgCmd::KeypadLed)
        return KeypadLedMsg(conf, rx, tx);
    else if(cmd == MsgCmd::KeypadButton)
        return KeypadButton::ProcessSettingsMsg(conf, rx, tx);
    else if(cmd == MsgCmd::KeypadButtonLed)
        return KeypadButton::ProcessSettingsMsg(conf, rx, tx);
    else if(cmd == MsgCmd::KeypadDial)
        return KeypadDial::ProcessSettingsMsg(conf, rx, tx);
    return MsgCmdResult::Invalid;
}

void Keypad::SetDefaultConfig(Config_Keypad *config)
{
    config->bEnabled = false;
    config->nNodeId = 0x15;
    config->bTimeoutEnabled = false;
    config->nTimeout = 20;
    config->eModel = KeypadModel::Blink12Key;
    config->nBacklightBrightness = 63;
    config->nDimBacklightBrightness = 31;
    config->nBacklightColor = (uint8_t)BlinkMarineBacklightColor::White;
    config->nDimmingVar = 0;
    config->nButtonBrightness = 63;
    config->nDimButtonBrightness = 31;
    
    for (uint8_t j = 0; j < KEYPAD_MAX_BUTTONS; j++)
    {
        KeypadButton::SetDefaultConfig(&config->stButton[j]);
    }

    for (uint8_t j = 0; j < KEYPAD_MAX_DIALS; j++)
    {
        KeypadDial::SetDefaultConfig(&config->stDial[j]);
    }
}