#include "virtual_input.h"

void VirtualInput::Update()
{
    if (!pConfig->bEnabled)
    {
        nVal = 0;
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
        nVal = input.Check(pConfig->eMode, false, bResultSec0);
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

    nVal = input.Check(pConfig->eMode, false, bResultSec1);
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
            nIndex = rx->data8[2];
        else
            nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_VIRT_INPUTS)
        {
            if (rx->DLC == 7)
            {
                conf->stVirtualInput[nIndex].bEnabled = (rx->data8[1] & 0x01);
                conf->stVirtualInput[nIndex].bNot0 = (rx->data8[1] & 0x02) >> 1;
                conf->stVirtualInput[nIndex].bNot1 = (rx->data8[1] & 0x04) >> 2;
                conf->stVirtualInput[nIndex].bNot2 = (rx->data8[1] & 0x08) >> 3;
                conf->stVirtualInput[nIndex].nVar0 = rx->data8[3];
                conf->stVirtualInput[nIndex].nVar1 = rx->data8[4];
                conf->stVirtualInput[nIndex].nVar2 = rx->data8[5];
                conf->stVirtualInput[nIndex].eCond0 = static_cast<BoolOperator>((rx->data8[6] & 0x03));
                conf->stVirtualInput[nIndex].eCond1 = static_cast<BoolOperator>((rx->data8[6] & 0x0C) >> 2);
                conf->stVirtualInput[nIndex].eMode = static_cast<InputMode>((rx->data8[6] & 0xC0) >> 6);
            }

            tx->DLC = 7;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::VirtualInputs) + 128;
            tx->data8[1] = ((conf->stVirtualInput[nIndex].bNot2 & 0x01) << 3) +
                          ((conf->stVirtualInput[nIndex].bNot1 & 0x01) << 2) +
                          ((conf->stVirtualInput[nIndex].bNot0 & 0x01) << 1) +
                          (conf->stVirtualInput[nIndex].bEnabled & 0x01);
            tx->data8[2] = nIndex;
            tx->data8[3] = conf->stVirtualInput[nIndex].nVar0;
            tx->data8[4] = conf->stVirtualInput[nIndex].nVar1;
            tx->data8[5] = conf->stVirtualInput[nIndex].nVar2;
            tx->data8[6] = ((static_cast<uint8_t>(conf->stVirtualInput[nIndex].eMode) & 0x0F) << 6) +
                          ((static_cast<uint8_t>(conf->stVirtualInput[nIndex].eCond1) & 0x03) << 2) +
                          (static_cast<uint8_t>(conf->stVirtualInput[nIndex].eCond0) & 0x03);
            tx->data8[7] = 0;

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
