/*
 * pdm_input.c
 *
 *  Created on: Jan 5, 2021
 *      Author: coryg
 */

#include "pdm_input.h"

void EvaluateInput(PdmConfig_Input_t *pIn, uint16_t* pResult)
{
  if(!pIn->nEnabled)
    return;

  uint16_t nLogicResult;

  nLogicResult = *pIn->pInput > pIn->nOnLevel;

  CheckPushbutton(&pIn->ePbConfig, pIn->eMode, nLogicResult, pResult, pIn->nDebounceTime);
}
