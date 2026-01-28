#include "keypad_dial.h"
#include "dbc.h"

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
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 3);
        uint8_t nDialIndex = Dbc::DecodeInt(rx->data8, 11, 5);

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 5)
            {
                conf->stKeypad[nIndex].stDial[nDialIndex].nDialMinLed = Dbc::DecodeInt(rx->data8, 16, 8);
                conf->stKeypad[nIndex].stDial[nDialIndex].nDialMaxLed = Dbc::DecodeInt(rx->data8, 24, 8);
                conf->stKeypad[nIndex].stDial[nDialIndex].nDialLedOffset = Dbc::DecodeInt(rx->data8, 32, 8);
            }

            tx->DLC = 5;
            tx->IDE = CAN_IDE_STD;
            for (int i = 0; i < 8; i++) tx->data8[i] = 0;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::KeypadDial) + 128;
            Dbc::EncodeInt(tx->data8, nIndex, 8, 3);
            Dbc::EncodeInt(tx->data8, nDialIndex, 11, 5);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stDial[nDialIndex].nDialMinLed, 16, 8);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stDial[nDialIndex].nDialMaxLed, 24, 8);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stDial[nDialIndex].nDialLedOffset, 32, 8);

            if(rx->DLC == 5)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

void KeypadDial::SetDefaultConfig(Config_KeypadDial *config)
{
    config->nDialMinLed = 0;
    config->nDialMaxLed = 0;
    config->nDialLedOffset = 0;
}