#include "flasher.h"
#include "dbc.h"

void Flasher::Update(uint32_t nTimeNow)
{
    if (!pConfig->bEnabled)
    {
        fVal = 0;
        return;
    }

    if (!*pInput)
    {
        fVal = 0;
        return;
    }
    
    if ((fVal == 0) && ((nTimeNow - nTimeOff) > pConfig->nFlashOffTime))
    {
        fVal = 1;
        nTimeOn = nTimeNow;
    }
    if ((fVal == 1) && ((nTimeNow - nTimeOn) > pConfig->nFlashOnTime))
    {
        fVal = 0;
        nTimeOff = nTimeNow;
    }
}

MsgCmdResult Flasher::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 6 = Set flasher settings
    // DLC 2 = Get flasher settings

    if ((rx->DLC == 6) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 12, 4);
        if (nIndex < PDM_NUM_FLASHERS)
        {
            if (rx->DLC == 6)
            {
                conf->stFlasher[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 8, 1);
                conf->stFlasher[nIndex].bSingleCycle = Dbc::DecodeInt(rx->data8, 9, 1);
                conf->stFlasher[nIndex].nInput = Dbc::DecodeInt(rx->data8, 16, 16);
                conf->stFlasher[nIndex].nFlashOnTime = Dbc::DecodeInt(rx->data8, 32, 8, 100.0f);
                conf->stFlasher[nIndex].nFlashOffTime = Dbc::DecodeInt(rx->data8, 40, 8, 100.0f);
            }

            tx->DLC = 6;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++) tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::Flashers) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, conf->stFlasher[nIndex].bEnabled, 8, 1);
            Dbc::EncodeInt(tx->data8, conf->stFlasher[nIndex].bSingleCycle, 9, 1);
            Dbc::EncodeInt(tx->data8, nIndex, 12, 4);
            Dbc::EncodeInt(tx->data8, conf->stFlasher[nIndex].nInput, 16, 16);
            Dbc::EncodeInt(tx->data8, conf->stFlasher[nIndex].nFlashOnTime, 32, 8, 100.0f);
            Dbc::EncodeInt(tx->data8, conf->stFlasher[nIndex].nFlashOffTime, 40, 8, 100.0f);

            if (rx->DLC == 6)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

void Flasher::SetDefaultConfig(Config_Flasher *config)
{
    config->bEnabled = false;
    config->nInput = 0;
    config->nFlashOnTime = 500;
    config->nFlashOffTime = 500;
    config->bSingleCycle = false;
}