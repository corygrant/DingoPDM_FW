#include "starter.h"

void Starter::Update()
{
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        if (!pConfig->bEnabled)
            nVal[i] = 1;
        else
            nVal[i] = !(pConfig->bDisableOut[i] && *pInput);
    }
}

MsgCmdResult Starter::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 4 = Set starter settings
    // DLC 1 = Get starter settings

    if ((rx->DLC == 4) ||
        (rx->DLC == 1))
    {
        if (rx->DLC == 4)
        {
            conf->stStarter.bEnabled = (rx->data8[1] & 0x01);
            conf->stStarter.nInput = rx->data8[2];
            conf->stStarter.bDisableOut[0] = (rx->data8[3] & 0x01);
            conf->stStarter.bDisableOut[1] = (rx->data8[3] & 0x02) >> 1;
            conf->stStarter.bDisableOut[2] = (rx->data8[3] & 0x04) >> 2;
            conf->stStarter.bDisableOut[3] = (rx->data8[3] & 0x08) >> 3;
            conf->stStarter.bDisableOut[4] = (rx->data8[3] & 0x10) >> 4;
            conf->stStarter.bDisableOut[5] = (rx->data8[3] & 0x20) >> 5;
            conf->stStarter.bDisableOut[6] = (rx->data8[3] & 0x40) >> 6;
            conf->stStarter.bDisableOut[7] = (rx->data8[3] & 0x80) >> 7;
        }

        tx->DLC = 4;
        tx->IDE = CAN_IDE_STD;

        tx->data8[0] = static_cast<uint8_t>(MsgCmd::StarterDisable) + 128;
        tx->data8[1] = (conf->stStarter.bEnabled & 0x01);
        tx->data8[2] = conf->stStarter.nInput;
        tx->data8[3] = ((conf->stStarter.bDisableOut[7] & 0x01) << 7) +
                      ((conf->stStarter.bDisableOut[6] & 0x01) << 6) +
                      ((conf->stStarter.bDisableOut[5] & 0x01) << 5) +
                      ((conf->stStarter.bDisableOut[4] & 0x01) << 4) +
                      ((conf->stStarter.bDisableOut[3] & 0x01) << 3) +
                      ((conf->stStarter.bDisableOut[2] & 0x01) << 2) +
                      ((conf->stStarter.bDisableOut[1] & 0x01) << 1) +
                      (conf->stStarter.bDisableOut[0] & 0x01);
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