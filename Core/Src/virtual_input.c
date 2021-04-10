/*
 * virtual_input.c
 *
 *  Created on: Jan 6, 2021
 *      Author: coryg
 */

#include "virtual_input.h"

void EvaluateVirtInput(PdmConfig_VirtualInput_t *pIn, uint16_t* pResult)
{
  if(!pIn->nEnabled)
    return;
  if((pIn->pVar0 == 0) || (pIn->pVar1 == 0))
    return;

  uint8_t nResult0, nResult1, nResult2, nResultSec0;

  nResult0 = *pIn->pVar0;
  if(pIn->nNot0)
    nResult0 = !nResult0;

  nResult1 = *pIn->pVar1;
  if(pIn->nNot1)
    nResult1 = !nResult1;

  switch(pIn->eCond0)
  {
  case COND_AND:
    nResultSec0 = nResult0 && nResult1;
    break;
  case COND_OR:
    nResultSec0 = nResult0 || nResult1;
    break;
  case COND_NOR:
    nResultSec0 = !nResult0 || !nResult1;
    break;
  }

  //Only 2 conditions
  if(pIn->nVar2 == 0)
  {
    CheckPushbutton(&pIn->ePbConfig, pIn->eMode, nResultSec0, pResult, NO_DEBOUNCE);
    return;
  }
  else
  {
    nResult2 = *pIn->pVar2;
    if(pIn->nNot2)
      nResult2 = !nResult2;

    switch(pIn->eCond0)
    {
    case COND_AND:
      CheckPushbutton(&pIn->ePbConfig, pIn->eMode, nResultSec0 && nResult2, pResult, NO_DEBOUNCE);
      return;
    case COND_OR:
      CheckPushbutton(&pIn->ePbConfig, pIn->eMode, nResultSec0 || nResult2, pResult, NO_DEBOUNCE);
      return;
    case COND_NOR:
      CheckPushbutton(&pIn->ePbConfig, pIn->eMode, !nResultSec0 || !nResult2, pResult, NO_DEBOUNCE);
      return;
    }
  }

}
