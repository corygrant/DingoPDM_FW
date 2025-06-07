#include "condition.h"

void Condition::Update()
{
    if (!pConfig->bEnabled)
    {
        nVal = 0;
        return;
    }

    switch(pConfig->eOperator)
    {
        case Operator::Equal:
            nVal = *pInput == pConfig->nArg;
            break;

        case Operator::NotEqual:
            nVal = *pInput != pConfig->nArg;
            break;

        case Operator::GreaterThan:
            nVal = *pInput > pConfig->nArg;
            break;

        case Operator::LessThan:
            nVal = *pInput < pConfig->nArg;
            break;

        case Operator::GreaterThanOrEqual:
            nVal = *pInput >= pConfig->nArg;
            break;

        case Operator::LessThanOrEqual:
            nVal = *pInput <= pConfig->nArg;
            break;

        case Operator::BitwiseAnd:
            nVal = *pInput & pConfig->nArg;
            break;

        case Operator::BitwiseNand: 
            nVal = ~(*pInput & pConfig->nArg);
            break;

        default:
            nVal = 0;
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
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_CONDITIONS)
        {
            if (rx->DLC == 6)
            {
                conf->stCondition[nIndex].bEnabled = (rx->data8[2] & 0x01);
                conf->stCondition[nIndex].eOperator = static_cast<Operator>(rx->data8[2] >> 4);
                conf->stCondition[nIndex].nInput = rx->data8[3];
                conf->stCondition[nIndex].nArg = (rx->data8[4] << 8) + rx->data8[5];
            }

            tx->DLC = 6;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::Conditions) + 128;
            tx->data8[1] = nIndex;
            tx->data8[2] = (conf->stCondition[nIndex].bEnabled & 0x01) +
                           ((static_cast<uint8_t>(conf->stCondition[nIndex].eOperator) & 0x0F) << 4);
            tx->data8[3] = conf->stCondition[nIndex].nInput;
            tx->data8[4] = (conf->stCondition[nIndex].nArg & 0xFF00) >> 8;
            tx->data8[5] = conf->stCondition[nIndex].nArg & 0x00FF;

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
    config->nArg = 0;
}