#include "can_input.h"

uint8_t EvaluateCANInput(CAN_RxHeaderTypeDef* stRxHeader, uint8_t nRxData[8], PdmConfig_CanInput_t *stIn, uint16_t* nResult)
{
  if(!stIn->nEnabled)
    return 0;
  if(stRxHeader->StdId != stIn->nId)
    return 0;

  uint16_t nSelected;

  //8 bit
  if(stIn->nHighByte == 0)
  {
    nSelected = nRxData[stIn->nLowByte];
  }
  else
  {
    nSelected = (nRxData[stIn->nHighByte] << 8) + nRxData[stIn->nLowByte];
  }

  switch(stIn->eOperator)
  {
  case OPER_EQUAL:
    *nResult = (nSelected == stIn->nOnVal);
    break;

  case OPER_GREATER_THAN:
    *nResult = nSelected > stIn->nOnVal;
    break;

  case OPER_LESS_THAN:
    *nResult = nSelected < stIn->nOnVal;
    break;

  case OPER_BITWISE_AND:
    if (stIn->eMode == MODE_NUM)
      *nResult = (nSelected & stIn->nOnVal);
    else
      CheckInput(&stIn->stInVars, stIn->eMode, false, ((nSelected & stIn->nOnVal) > 0), nResult, NO_DEBOUNCE);
    break;

  case OPER_BITWISE_NAND:
    if (stIn->eMode == MODE_NUM)
      *nResult = (nSelected & !stIn->nOnVal);
    else
      CheckInput(&stIn->stInVars, stIn->eMode, false, !((nSelected & stIn->nOnVal) > 0), nResult, NO_DEBOUNCE);
    break;
  }

  return 1;

}

