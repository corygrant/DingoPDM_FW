#include "virtual_input.h"
#include "dbc.h"

void VirtualInput::Update()
{
    if (!pConfig->bEnabled)
    {
        fVal = 0;
        return;
    }
    
    if ((pVar0 == 0) || (pVar1 == 0))
        return;

    bResult0 = *pVar0;
    if (pConfig->bNot0)
        bResult0 = !bResult0;

    bResult1 = *pVar1;
    if (pConfig->bNot1)
        bResult1 = !bResult1;

    switch (pConfig->eCond0)
    {
    case BoolOperator::And:
        bResultSec0 = bResult0 && bResult1;
        break;
    case BoolOperator::Or:
        bResultSec0 = bResult0 || bResult1;
        break;
    case BoolOperator::Nor:
        bResultSec0 = !bResult0 || !bResult1;
        break;
    }

    // Only 2 conditions
    if (pConfig->nVar2 == 0)
    {
        fVal = input.Check(pConfig->eMode, false, bResultSec0);
        return;
    }

    bResult2 = *pVar2;

    if (pConfig->bNot2)
        bResult2 = !bResult2;

    switch (pConfig->eCond1)
    {
    case BoolOperator::And:
        bResultSec1 = bResultSec0 && bResult2;
        break;
    case BoolOperator::Or:
        bResultSec1 = bResultSec0 || bResult2;
        break;
    case BoolOperator::Nor:
        bResultSec1 = !bResultSec0 || !bResult2;
        break;
    }

    fVal = input.Check(pConfig->eMode, false, bResultSec1);
}

MsgCmdResult VirtualInput::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 7 = Set virtual input settings
    // DLC 2 = Get virtual input settings

    if ((rx->DLC == 7) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex;
        if (rx->DLC == 7)
            nIndex = Dbc::DecodeInt(rx->data8, 16, 8);
        else
            nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_VIRT_INPUTS)
        {
            if (rx->DLC == 7)
            {
                conf->stVirtualInput[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 8, 1);
                conf->stVirtualInput[nIndex].bNot0 = Dbc::DecodeInt(rx->data8, 9, 1);
                conf->stVirtualInput[nIndex].bNot1 = Dbc::DecodeInt(rx->data8, 10, 1);
                conf->stVirtualInput[nIndex].bNot2 = Dbc::DecodeInt(rx->data8, 11, 1);
                conf->stVirtualInput[nIndex].nVar0 = Dbc::DecodeInt(rx->data8, 24, 8);
                conf->stVirtualInput[nIndex].nVar1 = Dbc::DecodeInt(rx->data8, 32, 8);
                conf->stVirtualInput[nIndex].nVar2 = Dbc::DecodeInt(rx->data8, 40, 8);
                conf->stVirtualInput[nIndex].eCond0 = static_cast<BoolOperator>(Dbc::DecodeInt(rx->data8, 48, 2));
                conf->stVirtualInput[nIndex].eCond1 = static_cast<BoolOperator>(Dbc::DecodeInt(rx->data8, 50, 2));
                conf->stVirtualInput[nIndex].eMode = static_cast<InputMode>(Dbc::DecodeInt(rx->data8, 54, 2));
            }

            tx->DLC = 7;
            tx->IDE = CAN_IDE_STD;

            // Clear tx data
            for (int i = 0; i < 8; i++)
                tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::VirtualInputs) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bEnabled, 8, 1);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bNot0, 9, 1);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bNot1, 10, 1);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bNot2, 11, 1);
            Dbc::EncodeInt(tx->data8, nIndex, 16, 8);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].nVar0, 24, 8);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].nVar1, 32, 8);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].nVar2, 40, 8);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stVirtualInput[nIndex].eCond0), 48, 2);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stVirtualInput[nIndex].eCond1), 50, 2);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stVirtualInput[nIndex].eMode), 54, 2);

            if (rx->DLC == 7)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}

void VirtualInput::SetDefaultConfig(Config_VirtualInput *config)
{
    config->bEnabled = false;
    config->bNot0 = false;
    config->nVar0 = 0;
    config->eCond0 = BoolOperator::And;
    config->bNot1 = false;
    config->nVar1 = 0;
    config->eCond1 = BoolOperator::And;
    config->bNot2 = false;
    config->nVar2 = 0;
    config->eMode = InputMode::Momentary;
}
