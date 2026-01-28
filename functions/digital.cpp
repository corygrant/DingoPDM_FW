#include "digital.h"
#include "dbc.h"
#include "input.h"

void Digital::Update()
{
    if(!pConfig->bEnabled)
    {
        fVal = 0;
        return;
    }

    bool bIn;

    bIn = palReadLine(m_line);

    // Debounce input
    if (bIn != bLast)
    {
        nLastTrigTime = SYS_TIME;
        bCheck = true;
    }

    bLast = bIn;

    if ((bCheck && ((SYS_TIME - nLastTrigTime) > pConfig->nDebounceTime)) || (!bInit))
    {
        bCheck = false;
        fVal = input.Check(pConfig->eMode, pConfig->bInvert, bIn);
    }

    bInit = true;
}

void Digital::SetPull(InputPull pull)
{
    switch (pull)
    {
    case InputPull::None:
        palSetLineMode(m_line, PAL_MODE_INPUT);
        break;
    case InputPull::Up:
        palSetLineMode(m_line, PAL_MODE_INPUT_PULLUP);
        break;
    case InputPull::Down:
        palSetLineMode(m_line, PAL_MODE_INPUT_PULLDOWN);
        break;
    }
}

MsgCmdResult Digital::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 4 = Set input settings
    // DLC 2 = Get input settings

    if ((rx->DLC == 4) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 12, 4);
        if (nIndex < PDM_NUM_INPUTS)
        {

            if (rx->DLC == 4)
            {
                conf->stInput[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 8, 1);
                conf->stInput[nIndex].eMode = static_cast<InputMode>(Dbc::DecodeInt(rx->data8, 9, 2));
                conf->stInput[nIndex].bInvert = Dbc::DecodeInt(rx->data8, 11, 1);
                conf->stInput[nIndex].nDebounceTime = Dbc::DecodeInt(rx->data8, 16, 8, 10.0f);
                conf->stInput[nIndex].ePull = static_cast<InputPull>(Dbc::DecodeInt(rx->data8, 24, 2));
            }

            tx->DLC = 4;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++) tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::Inputs) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, conf->stInput[nIndex].bEnabled, 8, 1);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stInput[nIndex].eMode), 9, 2);
            Dbc::EncodeInt(tx->data8, conf->stInput[nIndex].bInvert, 11, 1);
            Dbc::EncodeInt(tx->data8, nIndex, 12, 4);
            Dbc::EncodeInt(tx->data8, conf->stInput[nIndex].nDebounceTime, 16, 8, 10.0f);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stInput[nIndex].ePull), 24, 2);

            if (rx->DLC == 4)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}