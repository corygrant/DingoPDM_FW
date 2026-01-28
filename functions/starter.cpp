#include "starter.h"
#include "dbc.h"

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
            conf->stStarter.bEnabled = Dbc::DecodeInt(rx->data8, 8, 1);
            conf->stStarter.nInput = Dbc::DecodeInt(rx->data8, 16, 8);
            for(int i = 0; i < PDM_NUM_OUTPUTS && i < 8; i++) {
                conf->stStarter.bDisableOut[i] = Dbc::DecodeInt(rx->data8, 24 + i, 1);
            }
        }

        tx->DLC = 4;
        tx->IDE = CAN_IDE_STD;

        for (int i = 0; i < 8; i++) tx->data8[i] = 0;

        Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::StarterDisable) + 128, 0, 8);
        Dbc::EncodeInt(tx->data8, conf->stStarter.bEnabled, 8, 1);
        Dbc::EncodeInt(tx->data8, conf->stStarter.nInput, 16, 8);
        for(int i = 0; i < PDM_NUM_OUTPUTS && i < 8; i++) {
            Dbc::EncodeInt(tx->data8, conf->stStarter.bDisableOut[i], 24 + i, 1);
        }

        if (rx->DLC == 4)
            return MsgCmdResult::Write;
        else
            return MsgCmdResult::Request;
    }
    return MsgCmdResult::Invalid;
}

void Starter::SetDefaultConfig(Config_Starter *config)
{
    config->bEnabled = false;
    config->nInput = 0;
    for(uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        config->bDisableOut[i] = false;
    }
}