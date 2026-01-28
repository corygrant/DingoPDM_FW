#include "can_input.h"
#include "dbc.h"

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
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_CAN_INPUTS)
        {
            if (rx->DLC == 7)
            {
                conf->stCanInput[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 16, 1);
                conf->stCanInput[nIndex].eMode = static_cast<InputMode>(Dbc::DecodeInt(rx->data8, 17, 2));
                conf->stCanInput[nIndex].bTimeoutEnabled = Dbc::DecodeInt(rx->data8, 19, 1);
                conf->stCanInput[nIndex].eOperator = static_cast<Operator>(Dbc::DecodeInt(rx->data8, 20, 4));

                conf->stCanInput[nIndex].nStartingByte = Dbc::DecodeInt(rx->data8, 24, 4);
                conf->stCanInput[nIndex].nDLC = Dbc::DecodeInt(rx->data8, 28, 4);

                conf->stCanInput[nIndex].nOnVal = Dbc::DecodeInt(rx->data8, 32, 16);

                conf->stCanInput[nIndex].nTimeout = Dbc::DecodeInt(rx->data8, 48, 8, 100.0f);
            }

            tx->DLC = 7;
            tx->IDE = CAN_IDE_STD;

            // Clear tx data
            for (int i = 0; i < 8; i++)
                tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::CanInputs) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].bEnabled, 16, 1);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCanInput[nIndex].eMode), 17, 2);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].bTimeoutEnabled, 19, 1);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCanInput[nIndex].eOperator), 20, 4);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nStartingByte, 24, 4);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nDLC, 28, 4);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nOnVal, 32, 16);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nTimeout, 48, 8, 100.0f);

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
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_CAN_INPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stCanInput[nIndex].nIDE = Dbc::DecodeInt(rx->data8, 19, 1);
                conf->stCanInput[nIndex].nSID = Dbc::DecodeInt(rx->data8, 16, 11);
                conf->stCanInput[nIndex].nEID = Dbc::DecodeInt(rx->data8, 32, 29);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            // Clear tx data
            for (int i = 0; i < 8; i++)
                tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::CanInputsId) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nSID, 16, 11);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nIDE, 19, 1);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nEID, 32, 29);

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