#include "blink_keypad.h"
#include "blink_button.h"
#include "keypad.h"

namespace {

bool ColorToRed(BlinkMarineButtonColor eColor)
{
    return (static_cast<uint8_t>(eColor) & 0x01);
}

bool ColorToGreen(BlinkMarineButtonColor eColor)
{
    return ((static_cast<uint8_t>(eColor) & 0x02) >> 1);
}

bool ColorToBlue(BlinkMarineButtonColor eColor)
{
    return ((static_cast<uint8_t>(eColor) & 0x04) >> 2);
}

uint64_t BuildLedMsg(Keypad* kp, bool bBlink)
{
    uint64_t nMsg = 0;
    // Blink Marine PKP-2600SI 12 button keypad
    // Has different LED mapping than other keypads
    // Stacked
    if (kp->pConfig->eModel == KeypadModel::Blink12Key)
    {
        uint8_t nIndex = 0;
        for (uint8_t i = 0; i < kp->nNumButtons; i++)
        {
            nMsg |= (ColorToRed(bBlink ? kp->button[i].eLedBlinkColor : kp->button[i].eLedOnColor) << nIndex++);
        }
        for (uint8_t i = 0; i < kp->nNumButtons; i++)
        {
            nMsg |= (ColorToGreen(bBlink ? kp->button[i].eLedBlinkColor : kp->button[i].eLedOnColor) << nIndex++);
        }
        for (uint8_t i = 0; i < kp->nNumButtons; i++)
        {
            nMsg |= (ColorToBlue(bBlink ? kp->button[i].eLedBlinkColor : kp->button[i].eLedOnColor) << nIndex++);
        }

        return nMsg;
    }

    // All other Blink keypads have padding
    // Padded
    uint8_t nBytesPerColor = (kp->nNumButtons + 7) / 8;  // Ceiling
    uint8_t nByteIndex = 0;
    uint8_t nBitIndex = 0;
    uint8_t nBitPosition = 0;

    for (uint8_t i = 0; i < kp->nNumButtons; i++) {
        nByteIndex = i / 8;
        nBitIndex = i % 8;
        nBitPosition = (nByteIndex * 8) + nBitIndex;
        nMsg |= (ColorToRed(bBlink ? kp->button[i].eLedBlinkColor : kp->button[i].eLedOnColor) << nBitPosition);
    }

    for (uint8_t i = 0; i < kp->nNumButtons; i++) {
        nByteIndex = nBytesPerColor + (i / 8);
        nBitIndex = i % 8;
        nBitPosition = (nByteIndex * 8) + nBitIndex;
        nMsg |= (ColorToGreen(bBlink ? kp->button[i].eLedBlinkColor : kp->button[i].eLedOnColor) << nBitPosition);
    }

    for (uint8_t i = 0; i < kp->nNumButtons; i++) {
        nByteIndex = 2 * nBytesPerColor + (i / 8);
        nBitIndex = i % 8;
        nBitPosition = (nByteIndex * 8) + nBitIndex;
        nMsg |= (ColorToBlue(bBlink ? kp->button[i].eLedBlinkColor : kp->button[i].eLedOnColor) << nBitPosition);
    }

    return nMsg;
}

CANTxFrame LedOnMsg(Keypad* kp)
{
    CANTxFrame msg;
    msg.SID = kp->pConfig->nNodeId + 0x200;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data64[0] = BuildLedMsg(kp, false);

    return msg;
}

CANTxFrame LedBlinkMsg(Keypad* kp)
{
    CANTxFrame msg;
    msg.SID = kp->pConfig->nNodeId + 0x300;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data64[0] = BuildLedMsg(kp, true);

    return msg;
}

CANTxFrame LedBrightnessMsg(Keypad* kp)
{
    CANTxFrame msg;
    msg.SID = kp->pConfig->nNodeId + 0x400;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data8[0] = *kp->pDimmingInput ? kp->pConfig->nDimButtonBrightness : kp->pConfig->nButtonBrightness;
    msg.data8[1] = 0x00;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}

CANTxFrame BacklightMsg(Keypad* kp)
{
    CANTxFrame msg;
    msg.SID = kp->pConfig->nNodeId + 0x500;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data8[0] = *kp->pDimmingInput ? kp->pConfig->nDimBacklightBrightness : kp->pConfig->nBacklightBrightness;
    msg.data8[1] = kp->pConfig->nBacklightColor;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}

}

void SetModelBlinkMarine(Keypad* kp)
{
    kp->nNumButtons = 0;
    kp->nNumDials = 0;
    kp->numAnalogInputs = 0;

    switch (kp->pConfig->eModel)
    {
        case KeypadModel::Blink2Key:
            kp->nNumButtons = 2;
            break;
        case KeypadModel::Blink4Key:
            kp->nNumButtons = 4;
            break;
        case KeypadModel::Blink5Key:
            kp->nNumButtons = 5;
            break;
        case KeypadModel::Blink6Key:
            kp->nNumButtons = 6;
            break;
        case KeypadModel::Blink8Key:
            kp->nNumButtons = 8;
            break;
        case KeypadModel::Blink10Key:
            kp->nNumButtons = 10;
            break;
        case KeypadModel::Blink12Key:
            kp->nNumButtons = 12;
            break;
        case KeypadModel::Blink15Key:
            kp->nNumButtons = 15;
            break;
        case KeypadModel::Blink15Key2Dial:
            kp->nNumButtons = 15;
            kp->nNumDials = 2;
            kp->numAnalogInputs = 4;
            break;
        default:
            break;
    }

    for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        kp->button[i].SetConfig(&kp->pConfig->stButton[i]);

    for (uint8_t i = 0; i < KEYPAD_MAX_DIALS; i++)
        kp->dial[i].SetConfig(&kp->pConfig->stDial[i]);

    // Button function pointer for BlinkMarine LED behavior
    for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        kp->button[i].SetBrand(UpdateButtonLedBlinkMarine);
}

bool CheckMsgBlinkMarine(Keypad* kp, CANRxFrame frame)
{

    switch (frame.SID - kp->pConfig->nNodeId)
    {
        case (uint16_t)BlinkMarineMessageId::ButtonState:
            for(uint8_t i = 0; i < kp->nNumButtons; i++)
            {
                kp->fButtonVal[i] = kp->button[i].UpdateState((frame.data8[i / 8] >> (i % 8)) & 0x01);
            }
            break;
    
        case (uint16_t)BlinkMarineMessageId::DialState1:
            kp->dial[0].CheckMsg(frame.data64[0]);
            break;

        case (uint16_t)BlinkMarineMessageId::DialState2:
            kp->dial[1].CheckMsg(frame.data64[0]);
            break;

        case (uint16_t)BlinkMarineMessageId::AnalogInput:
            for (uint8_t i = 0; i < kp->numAnalogInputs; i++)
            {
                uint16_t nRawVal = frame.data8[i * 2] + (frame.data8[i * 2 + 1] << 8);
                kp->fAnalogVal[i] = static_cast<float>(nRawVal) * 0.01;
            }
            break;

        default:
            return false;
    }

    kp->nLastRxTime = SYS_TIME;

    return true;
}

CANTxFrame GetTxMsgBlinkMarine(Keypad* kp, uint8_t nIndex)
{
    switch (nIndex)
    {
    case 0:
        return LedOnMsg(kp);
    case 1:
        return LedBlinkMsg(kp);
    case 2:
        return LedBrightnessMsg(kp);
    case 3:
        return BacklightMsg(kp);
    default:
        return {};
    }
}

CANTxFrame GetStartMsgBlinkMarine(Keypad* kp)
{
    CANTxFrame msg;
    msg.SID = 0x00;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data8[0] = 0x01;
    msg.data8[1] = kp->pConfig->nNodeId;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}