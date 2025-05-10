#include "keypad.h"
#include "dingopdm_config.h"

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
        msg = keypad->GetTxMsg(0);
        PostTxFrame(&msg);
        chThdSleepMilliseconds(KEYPAD_TX_MSG_SPLIT);
        msg = keypad->GetTxMsg(1);
        PostTxFrame(&msg);
        chThdSleepMilliseconds(KEYPAD_TX_MSG_SPLIT);
        msg = keypad->GetTxMsg(2);
        PostTxFrame(&msg);

        chThdSleepMilliseconds(KEYPAD_TX_MSG_DELAY);
    }
}

msg_t Keypad::Init()
{
    if (!pConfig->bEnabled)
        return MSG_OK;
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

    if (frame.SID != pConfig->nBaseId + 0x180)
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

    return true;
}


CANTxFrame Keypad::LedOnMsg()
{
    for (uint8_t i = 0; i < pConfig->nNumButtons; i++)
        ColorToRGB(i, button[i].GetColor());
        
    CANTxFrame msg;
    msg.SID = pConfig->nBaseId + 0x200;
    msg.IDE = CAN_IDE_STD;
    msg.DLC = 8;
    msg.data64[0] = 0; // Ensure the value is initialized to 0
    uint8_t nIndex = 0;
    for (uint8_t i = 0; i < pConfig->nNumButtons; i++)
    {
        msg.data64[0] |= (uint64_t(bBtnLedOnRed[i]) << nIndex++);
    }
    for (uint8_t i = 0; i < pConfig->nNumButtons; i++)
    {
        msg.data64[0] |= (uint64_t(bBtnLedOnGreen[i]) << nIndex++);
    }
    for (uint8_t i = 0; i < pConfig->nNumButtons; i++)
    {
        msg.data64[0] |= (uint64_t(bBtnLedOnBlue[i]) << nIndex++);
    }

    return msg;
}

CANTxFrame Keypad::LedBrightnessMsg()
{
    CANTxFrame msg;
    msg.SID = pConfig->nBaseId + 0x400;
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
    msg.SID = pConfig->nBaseId + 0x500;
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

CANTxFrame Keypad::GetTxMsg(uint8_t nIndex)
{
    CANTxFrame msg;

    switch (nIndex)
    {
    case 0:
        msg = LedOnMsg();
        break;
    case 1:
        msg = LedBrightnessMsg();
        break;
    case 2:
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
    msg.data8[1] = pConfig->nBaseId;
    msg.data8[2] = 0x00;
    msg.data8[3] = 0x00;
    msg.data8[4] = 0x00;
    msg.data8[5] = 0x00;
    msg.data8[6] = 0x00;
    msg.data8[7] = 0x00;

    return msg;
}

void Keypad::ColorToRGB(uint8_t nBtn, BlinkMarineButtonColor color)
{
    switch (color)
    {
    case BlinkMarineButtonColor::BTN_OFF:
        bBtnLedOnRed[nBtn] = false;
        bBtnLedOnGreen[nBtn] = false;
        bBtnLedOnBlue[nBtn] = false;
        break;
    case BlinkMarineButtonColor::BTN_RED:
        bBtnLedOnRed[nBtn] = true;
        bBtnLedOnGreen[nBtn] = false;
        bBtnLedOnBlue[nBtn] = false;
        break;
    case BlinkMarineButtonColor::BTN_GREEN:
        bBtnLedOnRed[nBtn] = false;
        bBtnLedOnGreen[nBtn] = true;
        bBtnLedOnBlue[nBtn] = false;
        break;
    case BlinkMarineButtonColor::BTN_ORANGE:
        bBtnLedOnRed[nBtn] = true;
        bBtnLedOnGreen[nBtn] = true;
        bBtnLedOnBlue[nBtn] = false;
        break;
    case BlinkMarineButtonColor::BTN_BLUE:
        bBtnLedOnRed[nBtn] = false;
        bBtnLedOnGreen[nBtn] = false;
        bBtnLedOnBlue[nBtn] = true;
        break;
    case BlinkMarineButtonColor::BTN_VIOLET:
        bBtnLedOnRed[nBtn] = true;
        bBtnLedOnGreen[nBtn] = false;
        bBtnLedOnBlue[nBtn] = true;
        break;
    case BlinkMarineButtonColor::BTN_CYAN:
        bBtnLedOnRed[nBtn] = false;
        bBtnLedOnGreen[nBtn] = true;
        bBtnLedOnBlue[nBtn] = true;
        break;
    case BlinkMarineButtonColor::BTN_WHITE:
        bBtnLedOnRed[nBtn] = true;
        bBtnLedOnGreen[nBtn] = true;
        bBtnLedOnBlue[nBtn] = true;
        break;
    default:
        break;
    }
}