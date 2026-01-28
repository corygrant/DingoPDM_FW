#include "digital.h"
#include "input.h"

void Digital::Update()
{
    if(!pConfig->bEnabled)
    {
        nVal = 0;
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
        nVal = input.Check(pConfig->eMode, pConfig->bInvert, bIn);
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
        uint8_t nIndex = (rx->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_INPUTS)
        {

            if (rx->DLC == 4)
            {
                conf->stInput[nIndex].bEnabled = (rx->data8[1] & 0x01);
                conf->stInput[nIndex].eMode = static_cast<InputMode>((rx->data8[1] & 0x06) >> 1);
                conf->stInput[nIndex].bInvert = (rx->data8[1] & 0x08) >> 3;
                conf->stInput[nIndex].nDebounceTime = rx->data8[2] * 10;
                conf->stInput[nIndex].ePull = static_cast<InputPull>(rx->data8[3] & 0x03);
            }

            tx->DLC = 4;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::Inputs) + 128;
            tx->data8[1] = ((nIndex & 0x0F) << 4) + ((conf->stInput[nIndex].bInvert & 0x01) << 3) +
                          ((static_cast<uint8_t>(conf->stInput[nIndex].eMode) & 0x03) << 1) + (conf->stInput[nIndex].bEnabled & 0x01);
            tx->data8[2] = (uint8_t)(conf->stInput[nIndex].nDebounceTime / 10);
            tx->data8[3] = static_cast<uint8_t>(conf->stInput[nIndex].ePull);
            tx->data8[4] = 0;
            tx->data8[5] = 0;
            tx->data8[6] = 0;
            tx->data8[7] = 0;

            if (rx->DLC == 4)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}