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

MsgCmdResult VirtualInputMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 3 = Set virtual input settings (without vars)
    // DLC 2 = Get virtual input settings

    if ((rx->DLC == 3) || (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_VIRT_INPUTS)
        {
            if (rx->DLC == 3)
            {
                conf->stVirtualInput[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 16, 1);
                conf->stVirtualInput[nIndex].bNot0 = Dbc::DecodeInt(rx->data8, 17, 1);
                conf->stVirtualInput[nIndex].bNot1 = Dbc::DecodeInt(rx->data8, 18, 1);
                conf->stVirtualInput[nIndex].bNot2 = Dbc::DecodeInt(rx->data8, 19, 1);
                conf->stVirtualInput[nIndex].eCond0 = static_cast<BoolOperator>(Dbc::DecodeInt(rx->data8, 20, 2));
                conf->stVirtualInput[nIndex].eCond1 = static_cast<BoolOperator>(Dbc::DecodeInt(rx->data8, 22, 2));
            }

            tx->DLC = 3;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++)
                tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::VirtualInputs) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bEnabled, 16, 1);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bNot0, 17, 1);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bNot1, 18, 1);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].bNot2, 19, 1);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stVirtualInput[nIndex].eCond0), 20, 2);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stVirtualInput[nIndex].eCond1), 22, 2);

            if (rx->DLC == 3)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}

MsgCmdResult VirtualInputVarsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set virtual input vars
    // DLC 2 = Get virtual input vars

    if ((rx->DLC == 8) || (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_VIRT_INPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stVirtualInput[nIndex].nVar0 = Dbc::DecodeInt(rx->data8, 16, 16);
                conf->stVirtualInput[nIndex].nVar1 = Dbc::DecodeInt(rx->data8, 32, 16);
                conf->stVirtualInput[nIndex].nVar2 = Dbc::DecodeInt(rx->data8, 48, 16);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++)
                tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::VirtualInputsVars) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].nVar0, 16, 16);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].nVar1, 32, 16);
            Dbc::EncodeInt(tx->data8, conf->stVirtualInput[nIndex].nVar2, 48, 16);

            if (rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}

MsgCmdResult VirtualInput::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    MsgCmd cmd = static_cast<MsgCmd>(rx->data8[0]);

    if (cmd == MsgCmd::VirtualInputs)
        return VirtualInputMsg(conf, rx, tx);
    else if (cmd == MsgCmd::VirtualInputsVars)
        return VirtualInputVarsMsg(conf, rx, tx);

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
