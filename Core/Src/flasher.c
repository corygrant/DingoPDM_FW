/*
 * flasher.c
 *
 *  Created on: Jan 6, 2021
 *      Author: coryg
 */

#include "flasher.h"

void EvaluateFlasher(PdmConfig_Flasher_t *pFlasher, uint16_t pResult[12])
{
  if(!pFlasher->nEnabled){
    pResult[pFlasher->nOutput] = 1;
    return;
  }
  if(!*pFlasher->pInput){
    pResult[pFlasher->nOutput] = 1;
    return;
  }

  if((pResult[pFlasher->nOutput] == 0) && ((HAL_GetTick() - pFlasher->nTimeOff) > pFlasher->nFlashOffTime)){
    pResult[pFlasher->nOutput] = 1;
    pFlasher->nTimeOn = HAL_GetTick();
  }
  if((pResult[pFlasher->nOutput] == 1) && ((HAL_GetTick() - pFlasher->nTimeOn) > pFlasher->nFlashOnTime)){
    pResult[pFlasher->nOutput] = 0;
    pFlasher->nTimeOff = HAL_GetTick();
  }

}
