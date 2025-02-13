#include "can_input.h"

bool CanInput::CheckMsg(CANRxFrame frame)
{
    if (!pConfig->bEnabled)
        return false;
    if (pConfig->nIDE && 
        (pConfig->nEID != frame.EID))
        return false;
    if (!pConfig->nIDE && 
        (pConfig->nSID != frame.SID))
        return false;
    if (pConfig->nDLC == 0)
        return false;

    nLastRxTime = SYS_TIME;

    nData = 0;
    for (int i = 0; i <= (pConfig->nDLC - 1); i++) {
        nData |= frame.data8[pConfig->nStartingByte + i] << (8 * ((pConfig->nDLC - 1) - i));
    }

    if(pConfig->eMode == InputMode::Num)
    {
        switch (pConfig->eOperator)
        {
        case Operator::Equal:
            nVal = nData == pConfig->nOnVal;
            break;

        case Operator::GreaterThan:
            nVal = nData > pConfig->nOnVal;
            break;

        case Operator::LessThan:
            nVal = nData < pConfig->nOnVal;
            break;

        case Operator::BitwiseAnd:
            nVal = (nData & pConfig->nOnVal);
            break;

        case Operator::BitwiseNand:
            nVal = (nData & !pConfig->nOnVal);
            break;
        }

        return true;
    }

    //Use Input class to enable momentary/latching
    //Inputs are treated as boolean
    switch (pConfig->eOperator)
    {
    case Operator::Equal:
        nVal = input.Check(pConfig->eMode, false, nData == pConfig->nOnVal);
        break;

    case Operator::GreaterThan:
        nVal = input.Check(pConfig->eMode, false, nData > pConfig->nOnVal);
        break;

    case Operator::LessThan:
        nVal = input.Check(pConfig->eMode, false, nData < pConfig->nOnVal);
        break;

    case Operator::BitwiseAnd:
        nVal = input.Check(pConfig->eMode, false, (nData & pConfig->nOnVal) > 0);
        break;

    case Operator::BitwiseNand:
        nVal = input.Check(pConfig->eMode, false, !((nData & pConfig->nOnVal) > 0));
        break;
    }

    return true;
}

void CanInput::CheckTimeout()
{
    if(!pConfig->bTimeoutEnabled)
        return;

    if (SYS_TIME - nLastRxTime > pConfig->nTimeout)
        nVal = 0;
}