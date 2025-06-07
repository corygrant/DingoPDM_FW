#include "can_input.h"

bool CanInput::CheckMsg(CANRxFrame rx)
{
    if (!pConfig->bEnabled)
        return false;
    if (pConfig->nIDE &&
        (pConfig->nEID != rx.EID))
        return false;
    if (!pConfig->nIDE &&
        (pConfig->nSID != rx.SID))
        return false;
    if (pConfig->nDLC == 0)
        return false;

    nLastRxTime = SYS_TIME;

    nData = 0;
    for (int i = 0; i <= (pConfig->nDLC - 1); i++)
    {
        nData |= rx.data8[pConfig->nStartingByte + i] << (8 * i);
    }

    //Copy data to shared value
    nVal = nData;

    // Use Input class to enable momentary/latching
    switch (pConfig->eOperator)
    {
    case Operator::Equal:
        nOutput = input.Check(pConfig->eMode, false, nData == pConfig->nOnVal);
        break;

    case Operator::NotEqual:
        nOutput = input.Check(pConfig->eMode, false, nData != pConfig->nOnVal);
        break;

    case Operator::GreaterThan:
        nOutput = input.Check(pConfig->eMode, false, nData > pConfig->nOnVal);
        break;

    case Operator::LessThan:
        nOutput = input.Check(pConfig->eMode, false, nData < pConfig->nOnVal);
        break;

    case Operator::GreaterThanOrEqual:
        nOutput = input.Check(pConfig->eMode, false, nData >= pConfig->nOnVal);
        break;

    case Operator::LessThanOrEqual:
        nOutput = input.Check(pConfig->eMode, false, nData <= pConfig->nOnVal);
        break;

    case Operator::BitwiseAnd:
        nOutput = input.Check(pConfig->eMode, false, (nData & pConfig->nOnVal) > 0);
        break;

    case Operator::BitwiseNand:
        nOutput = input.Check(pConfig->eMode, false, !((nData & pConfig->nOnVal) > 0));
        break;
    }

    return true;
}

void CanInput::CheckTimeout()
{
    if (!pConfig->bTimeoutEnabled)
        return;

    if (SYS_TIME - nLastRxTime > pConfig->nTimeout)
    {
        nVal = 0;
        nOutput = 0;
    }
}

MsgCmdResult CanInputMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 7 = Set CAN input settings
    // DLC 2 = Get CAN input settings

    if ((rx->DLC == 7) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_CAN_INPUTS)
        {
            if (rx->DLC == 7)
            {
                conf->stCanInput[nIndex].bEnabled = (rx->data8[2] & 0x01);
                conf->stCanInput[nIndex].eMode = static_cast<InputMode>((rx->data8[2] & 0x06) >> 1);
                conf->stCanInput[nIndex].bTimeoutEnabled = (rx->data8[2] & 0x08) >> 3;
                conf->stCanInput[nIndex].eOperator = static_cast<Operator>((rx->data8[2] & 0xF0) >> 4);

                conf->stCanInput[nIndex].nStartingByte = (rx->data8[3] & 0x0F);
                conf->stCanInput[nIndex].nDLC = (rx->data8[3] & 0xF0) >> 4;

                conf->stCanInput[nIndex].nOnVal = (rx->data8[4] << 8) + rx->data8[5];

                conf->stCanInput[nIndex].nTimeout = (rx->data8[6] * 100);
            }

            tx->DLC = 7;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::CanInputs) + 128;
            tx->data8[1] = nIndex;
            tx->data8[2] = ((static_cast<uint8_t>(conf->stCanInput[nIndex].eOperator) & 0x0F) << 4) +
                        ((static_cast<uint8_t>(conf->stCanInput[nIndex].eMode) & 0x03) << 1) +
                        ((conf->stCanInput[nIndex].bTimeoutEnabled & 0x01) << 3) +
                        (conf->stCanInput[nIndex].bEnabled & 0x01);
            tx->data8[3] = ((conf->stCanInput[nIndex].nDLC & 0xF) << 4) +
                            (conf->stCanInput[nIndex].nStartingByte & 0xF);
            tx->data8[4] = (uint8_t)(conf->stCanInput[nIndex].nOnVal >> 8);
            tx->data8[5] = (uint8_t)(conf->stCanInput[nIndex].nOnVal & 0xFF);
            tx->data8[6] = (uint8_t)(conf->stCanInput[nIndex].nTimeout / 100);

            if(rx->DLC == 7)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

MsgCmdResult CanInputIdMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set CAN input ID settings
    // DLC 2 = Get CAN input ID settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_CAN_INPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stCanInput[nIndex].nIDE = (rx->data8[2] & 0x08) >> 3;
                conf->stCanInput[nIndex].nSID = ((rx->data8[2] & 0x07) << 8) + rx->data8[3];
                conf->stCanInput[nIndex].nEID = ((rx->data8[4] & 0x1F) << 24) + (rx->data8[5] << 16) +
                                                   (rx->data8[6] << 8) + rx->data8[7];
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::CanInputsId) + 128;
            tx->data8[1] = nIndex;
            tx->data8[2] = ((conf->stCanInput[nIndex].nSID >> 8) & 0x07) +
                          ((conf->stCanInput[nIndex].nIDE & 0x01) << 3);
            tx->data8[3] = (uint8_t)(conf->stCanInput[nIndex].nSID & 0xFF);
            tx->data8[4] = (uint8_t)((conf->stCanInput[nIndex].nEID >> 24) & 0x1F);
            tx->data8[5] = (uint8_t)((conf->stCanInput[nIndex].nEID >> 16) & 0xFF);
            tx->data8[6] = (uint8_t)((conf->stCanInput[nIndex].nEID >> 8) & 0xFF);
            tx->data8[7] = (uint8_t)(conf->stCanInput[nIndex].nEID & 0xFF);

            if (rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}

MsgCmdResult CanInput::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    MsgCmd cmd = static_cast<MsgCmd>(rx->data8[0]);

    if(cmd == MsgCmd::CanInputs)
        return CanInputMsg(conf, rx, tx);
    else if(cmd == MsgCmd::CanInputsId)
        return CanInputIdMsg(conf, rx, tx);

    return MsgCmdResult::Invalid;
}

void CanInput::SetDefaultConfig(Config_CanInput *config)
{
    config->bEnabled = false;
    config->bTimeoutEnabled = true;
    config->nTimeout = 20;
    config->nIDE = 0;
    config->nSID = 0;
    config->nEID = 0;
    config->nDLC = 0;
    config->nStartingByte = 0;
    config->eOperator = Operator::Equal;
    config->nOnVal = 0;
    config->eMode = InputMode::Momentary;
}