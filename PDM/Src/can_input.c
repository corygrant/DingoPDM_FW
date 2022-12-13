/*
 * can_input.c
 *
 *  Created on: Jan 5, 2021
 *      Author: coryg
 */

#include "can_input.h"

uint8_t EvaluateCANInput(CAN_RxHeaderTypeDef* stRxHeader, uint8_t nRxData[8], PdmConfig_CanInput_t *in, uint16_t* nResult)
{
  if(!in->nEnabled)
    return 0;
  if(stRxHeader->StdId != in->nId)
    return 0;

  uint16_t nSelected;

  //8 bit
  if(in->nHighByte == 0)
  {
    nSelected = nRxData[in->nLowByte];
  }
  else
  {
    nSelected = (nRxData[in->nHighByte] << 8) + nRxData[in->nLowByte];
  }

  switch(in->eOperator)
  {
  case OPER_EQUAL:
    *nResult = nSelected & 0xFF;
    return 1;

  case OPER_GREATER_THAN:
    *nResult = nSelected > in->nOnVal;
    return 1;

  case OPER_LESS_THAN:
    *nResult = nSelected < in->nOnVal;
    return 1;

  case OPER_BITWISE_AND:
    if (in->eMode == MODE_NUM)
      *nResult = (nSelected & in->nOnVal);
    else
      CheckInput(&in->ePbConfig, in->eMode, ((nSelected & in->nOnVal) > 0), nResult, NO_DEBOUNCE);
    return 1;

  case OPER_BITWISE_NAND:
    if (in->eMode == MODE_NUM)
      *nResult = (nSelected & !in->nOnVal);
    else
      CheckInput(&in->ePbConfig, in->eMode, !((nSelected & in->nOnVal) > 0), nResult, NO_DEBOUNCE);
    return 1;
  }

  return 0;

}

