#include "condition.h"
#include "dbc.h"

void Condition::Update()
{
    if (!pConfig->bEnabled)
    {
        fVal = 0;
        return;
    }

    switch(pConfig->eOperator)
    {
        case Operator::Equal:
            fVal = *pInput == pConfig->fArg;
            break;

        case Operator::NotEqual:
            fVal = *pInput != pConfig->fArg;
            break;

        case Operator::GreaterThan:
            fVal = *pInput > pConfig->fArg;
            break;

        case Operator::LessThan:
            fVal = *pInput < pConfig->fArg;
            break;

        case Operator::GreaterThanOrEqual:
            fVal = *pInput >= pConfig->fArg;
            break;

        case Operator::LessThanOrEqual:
            fVal = *pInput <= pConfig->fArg;
            break;

        case Operator::BitwiseAnd:
            fVal = (uint16_t)(*pInput) & (uint16_t)(pConfig->fArg);
            break;

        case Operator::BitwiseNand:
            fVal = ~((uint16_t)(*pInput) & (uint16_t)(pConfig->fArg));
            break;

        default:
            fVal = 0;
            break;
    }
}

MsgCmdResult Condition::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 6 = Set condition settings
    // DLC 2 = Get condition settings

    if ((rx->DLC == 6) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 8);

        if (nIndex < PDM_NUM_CONDITIONS)
        {
            if (rx->DLC == 6)
            {
                conf->stCondition[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 16, 1);
                conf->stCondition[nIndex].eOperator = static_cast<Operator>(Dbc::DecodeInt(rx->data8, 20, 4));
                conf->stCondition[nIndex].nInput = Dbc::DecodeInt(rx->data8, 24, 8);
                conf->stCondition[nIndex].fArg = Dbc::DecodeFloat(rx->data8, 32, 16, 0.01f);
            }

            tx->DLC = 6;
            tx->IDE = CAN_IDE_STD;

            // Clear tx data
            for (int i = 0; i < 8; i++)
                tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::Conditions) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stCondition[nIndex].bEnabled, 16, 1);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCondition[nIndex].eOperator), 20, 4);
            Dbc::EncodeInt(tx->data8, conf->stCondition[nIndex].nInput, 24, 8);
            Dbc::EncodeFloat(tx->data8, conf->stCondition[nIndex].fArg, 32, 16, 0.01f);

            if(rx->DLC == 6)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

void Condition::SetDefaultConfig(Config_Condition *config)
{
    config->bEnabled = false;
    config->eOperator = Operator::Equal;
    config->nInput = 0;
    config->fArg = 0.0f;
}