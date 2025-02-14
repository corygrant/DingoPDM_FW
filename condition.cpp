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