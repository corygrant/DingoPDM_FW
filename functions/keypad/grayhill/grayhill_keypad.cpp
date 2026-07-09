#include "grayhill_keypad.h"
#include "grayhill_button.h"
#include "keypad.h"

namespace {

// Grayhill LED indicator control
// nNodeId + 0x200
CANTxFrame IndicatorMsg(Keypad* kp)
{
    CANTxFrame msg;
    msg.SID = kp->pConfig->nNodeId + 0x200;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data64[0] = 0;

    // Grayhill stacks LED indicators (3 per button) in order
    // Each LED state is 1 bit: 0 = off, 1 = on
    // Byte 0: Button 0 LED 0-2, Button 1 LED 0-2, Button 2 LED 0-1
    // Byte 1: Button 2 LED 2, Button 3 LED 0-2, Button 4 LED 0-2
    // etc., up to max of 20 buttons (60 LEDs) across 8 bytes

    uint8_t nBitPosition = 0;
    for (uint8_t i = 0; i < kp->nNumButtons; i++)
    {
        if (nBitPosition >= 64) break;

        uint8_t nLedValue = 0;
        if (kp->button[i].bLed[0]) nLedValue |= 1; // Bit 0
        if (kp->button[i].bLed[1]) nLedValue |= 2; // Bit 1
        if (kp->button[i].bLed[2]) nLedValue |= 4; // Bit 2

        msg.data64[0] |= ((uint64_t)nLedValue << nBitPosition);
        nBitPosition += 3;
    }

    return msg;
}

// Grayhill brightness control
// nNodeId + 0x300
// Byte 0: Button brightness (0-100%)
// Byte 1: Backlight brightness (0-100%)
CANTxFrame BrightnessMsg(Keypad* kp)
{
    CANTxFrame msg;
    msg.SID = kp->pConfig->nNodeId + 0x300;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data8[0] = *kp->pDimmingInput ? kp->pConfig->nDimButtonBrightness : kp->pConfig->nButtonBrightness;
    msg.data8[1] = *kp->pDimmingInput ? kp->pConfig->nDimBacklightBrightness : kp->pConfig->nBacklightBrightness;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}

}

void SetModelGrayhill(Keypad* kp)
{
    kp->nNumButtons = 0;
    kp->nNumDials = 0; // Grayhill keypads don't have dials
    kp->numAnalogInputs = 0; // Grayhill keypads don't have analog inputs

    switch (kp->pConfig->eModel)
    {
        case KeypadModel::Grayhill6Key:
            kp->nNumButtons = 6;
            break;
        case KeypadModel::Grayhill8Key:
            kp->nNumButtons = 8;
            break;
        case KeypadModel::Grayhill15Key:
            kp->nNumButtons = 15;
            break;
        case KeypadModel::Grayhill20Key:
            kp->nNumButtons = 20;
            break;
        default:
            break;
    }

    for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        kp->button[i].SetConfig(&kp->pConfig->stButton[i]);

    // Button function pointers for Grayhill LED behavior
    for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        kp->button[i].SetBrand(UpdateButtonLedGrayhill);
}

bool CheckMsgGrayhill(Keypad* kp, CANRxFrame frame)
{
    // Grayhill uses CANopen TPDO format
    // nNodeId + 0x180
    // Byte 0-2: Button states (up to 20 buttons)
    if (frame.SID != kp->pConfig->nNodeId + 0x180)
        return false;

    for(uint8_t i = 0; i < kp->nNumButtons; i++)
        kp->fButtonVal[i] = kp->button[i].UpdateState((frame.data8[i / 8] >> (i % 8)) & 0x01);

    kp->nLastRxTime = SYS_TIME;

    return true;
}

CANTxFrame GetTxMsgGrayhill(Keypad* kp, uint8_t nIndex)
{
    switch (nIndex)
    {
    case 0:
        return IndicatorMsg(kp);
    case 1:
        return BrightnessMsg(kp);
    default:
        return {};
    }
}

CANTxFrame GetStartMsgGrayhill(Keypad* kp)
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