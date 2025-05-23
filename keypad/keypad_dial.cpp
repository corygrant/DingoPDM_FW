#include "keypad_dial.h"

void KeypadDial::Update(uint64_t data)
{
    nTicks = data & 0x7F;
    bClockwise = (data & 0x80) >> 7 ? true : false;
    bCounterClockwise = (data & 0x80) >> 7 ? false : true;

    nEncoderTicks = (data & 0xFFFF00) >> 8;

    nMaxEncoderTicks = (data & 0xFF000000) >> 24;
}

void KeypadDial::UpdateLeds()
{
    //TODO: Implement LED update logic
}

MsgCmdResult KeypadDial::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 5 = Set keypad dial settings
    // DLC 2 = Get keypad dial settings

    if ((rx->DLC == 5) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = (rx->data8[1] & 0x07);
        uint8_t nDialIndex = (rx->data8[1] & 0xF8) >> 3;

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 5)
            {
                conf->stKeypad[nIndex].stDial[nDialIndex].nDialMinLed = rx->data8[2];
                conf->stKeypad[nIndex].stDial[nDialIndex].nDialMaxLed = rx->data8[3];
                conf->stKeypad[nIndex].stDial[nDialIndex].nDialLedOffset = rx->data8[4];
            }

            tx->DLC = 5;
            tx->IDE = CAN_IDE_STD;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::KeypadDial) + 128;
            tx->data8[1] = (nIndex & 0x07) + ((nDialIndex & 0x1F) << 3);

            tx->data8[2] = conf->stKeypad[nIndex].stDial[nDialIndex].nDialMinLed;
            tx->data8[3] = conf->stKeypad[nIndex].stDial[nDialIndex].nDialMaxLed;
            tx->data8[4] = conf->stKeypad[nIndex].stDial[nDialIndex].nDialLedOffset;

            if(rx->DLC == 5)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}