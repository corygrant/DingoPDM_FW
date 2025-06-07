#include "flasher.h"

void Flasher::Update(uint32_t nTimeNow)
{
    if (!pConfig->bEnabled)
    {
        nVal = 0;
        return;
    }

    if (!*pInput)
    {
        nVal = 0;
        return;
    }
    
    if ((nVal == 0) && ((nTimeNow - nTimeOff) > pConfig->nFlashOffTime))
    {
        nVal = 1;
        nTimeOn = nTimeNow;
    }
    if ((nVal == 1) && ((nTimeNow - nTimeOn) > pConfig->nFlashOnTime))
    {
        nVal = 0;
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
        uint8_t nIndex = (rx->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_FLASHERS)
        {
            if (rx->DLC == 6)
            {
                conf->stFlasher[nIndex].bEnabled = (rx->data8[1] & 0x01);
                conf->stFlasher[nIndex].bSingleCycle = (rx->data8[1] & 0x02) >> 1;
                conf->stFlasher[nIndex].nInput = rx->data8[2];

                conf->stFlasher[nIndex].nFlashOnTime = rx->data8[4] * 100;
                conf->stFlasher[nIndex].nFlashOffTime = rx->data8[5] * 100;
            }

            tx->DLC = 6;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::Flashers) + 128;
            tx->data8[1] = ((nIndex & 0x0F) << 4) +
                          ((conf->stFlasher[nIndex].bSingleCycle & 0x01) << 1) +
                          (conf->stFlasher[nIndex].bEnabled & 0x01);
            tx->data8[2] = conf->stFlasher[nIndex].nInput;

            tx->data8[4] = conf->stFlasher[nIndex].nFlashOnTime / 100;
            tx->data8[5] = conf->stFlasher[nIndex].nFlashOffTime / 100;
            tx->data8[6] = 0;
            tx->data8[7] = 0;

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