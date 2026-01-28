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
    if (pConfig->nBitLength == 0)
        return false;

    nLastRxTime = SYS_TIME;

    // Use DBC DecodeFloat to extract and scale the signal value
    fVal = Dbc::DecodeFloat(rx.data8, pConfig->nStartBit, pConfig->nBitLength,
                            pConfig->fScale, pConfig->fOffset,
                            pConfig->eByteOrder, pConfig->bSigned);

    // Use Input class to enable momentary/latching
    switch (pConfig->eOperator)
    {
    case Operator::Equal:
        fOutput = input.Check(pConfig->eMode, false, fVal == pConfig->fOnVal);
        break;

    case Operator::NotEqual:
        fOutput = input.Check(pConfig->eMode, false, fVal != pConfig->fOnVal);
        break;

    case Operator::GreaterThan:
        fOutput = input.Check(pConfig->eMode, false, fVal > pConfig->fOnVal);
        break;

    case Operator::LessThan:
        fOutput = input.Check(pConfig->eMode, false, fVal < pConfig->fOnVal);
        break;

    case Operator::GreaterThanOrEqual:
        fOutput = input.Check(pConfig->eMode, false, fVal >= pConfig->fOnVal);
        break;

    case Operator::LessThanOrEqual:
        fOutput = input.Check(pConfig->eMode, false, fVal <= pConfig->fOnVal);
        break;

    case Operator::BitwiseAnd:
        fOutput = input.Check(pConfig->eMode, false, ((uint32_t)fVal & (uint32_t)pConfig->fOnVal) > 0);
        break;

    case Operator::BitwiseNand:
        fOutput = input.Check(pConfig->eMode, false, !(((uint32_t)fVal & (uint32_t)pConfig->fOnVal) > 0));
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
        fVal = 0;
        fOutput = 0;
    }
}

MsgCmdResult CanInputMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set CAN input settings (Part 1: general settings)
    // DLC 2 = Get CAN input settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_CAN_INPUTS)
        {
            if (rx->DLC == 8)
            {
                conf->stCanInput[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 16, 1);
                conf->stCanInput[nIndex].eMode = static_cast<InputMode>(Dbc::DecodeInt(rx->data8, 17, 2));
                conf->stCanInput[nIndex].bTimeoutEnabled = Dbc::DecodeInt(rx->data8, 19, 1);
                conf->stCanInput[nIndex].eOperator = static_cast<Operator>(Dbc::DecodeInt(rx->data8, 20, 4));

                conf->stCanInput[nIndex].nStartBit = Dbc::DecodeInt(rx->data8, 24, 8);
                conf->stCanInput[nIndex].nBitLength = Dbc::DecodeInt(rx->data8, 32, 6);
                conf->stCanInput[nIndex].eByteOrder = static_cast<ByteOrder>(Dbc::DecodeInt(rx->data8, 38, 1));
                conf->stCanInput[nIndex].bSigned = Dbc::DecodeInt(rx->data8, 39, 1);

                conf->stCanInput[nIndex].nTimeout = Dbc::DecodeInt(rx->data8, 48, 8, 100.0f);
            }

            tx->DLC = 8;
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
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nStartBit, 24, 8);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nBitLength, 32, 6);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCanInput[nIndex].eByteOrder), 38, 1);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].bSigned, 39, 1);
            Dbc::EncodeInt(tx->data8, conf->stCanInput[nIndex].nTimeout, 48, 8, 100.0f);

            if(rx->DLC == 8)
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

MsgCmdResult CanInputScaleMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // This message can be sent in two sizes:
    // DLC 8 = Set/Get scale and offset
    // DLC 6 = Set/Get fOnVal
    // DLC 2 = Get (will return DLC 8 with scale/offset)

    if ((rx->DLC == 8) || (rx->DLC == 6) || (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_CAN_INPUTS)
        {
            if (rx->DLC == 8)
            {
                // Decode scale (16-bit fixed point with 0.001 resolution)
                conf->stCanInput[nIndex].fScale = Dbc::DecodeFloat(rx->data8, 16, 16, 0.001f);
                // Decode offset (16-bit fixed point with 0.01 resolution)
                conf->stCanInput[nIndex].fOffset = Dbc::DecodeFloat(rx->data8, 32, 16, 0.01f);
                // Decode fOnVal (16-bit fixed point with 0.01 resolution)
                conf->stCanInput[nIndex].fOnVal = Dbc::DecodeFloat(rx->data8, 48, 16, 0.01f);
            }
            else if (rx->DLC == 6)
            {
                // Just updating fOnVal
                conf->stCanInput[nIndex].fOnVal = Dbc::DecodeFloat(rx->data8, 16, 32);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            // Clear tx data
            for (int i = 0; i < 8; i++)
                tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::CanInputsScale) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeFloat(tx->data8, conf->stCanInput[nIndex].fScale, 16, 16, 0.001f);
            Dbc::EncodeFloat(tx->data8, conf->stCanInput[nIndex].fOffset, 32, 16, 0.01f);
            Dbc::EncodeFloat(tx->data8, conf->stCanInput[nIndex].fOnVal, 48, 16, 0.01f);

            if (rx->DLC >= 6)
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
    else if(cmd == MsgCmd::CanInputsScale)
        return CanInputScaleMsg(conf, rx, tx);

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
    config->nStartBit = 0;
    config->nBitLength = 0;
    config->fScale = 1.0f;      // Default to no scaling
    config->fOffset = 0.0f;     // Default to no offset
    config->eByteOrder = ByteOrder::LittleEndian;
    config->bSigned = false;
    config->eOperator = Operator::Equal;
    config->fOnVal = 0.0f;
    config->eMode = InputMode::Momentary;
}