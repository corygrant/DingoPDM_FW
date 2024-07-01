#include "flasher.h"

void EvaluateFlasher(uint32_t nNow, PdmConfig_Flasher_t* pFlasher, uint16_t* pResult)
{
  if(pFlasher->nEnabled == 0){
    *pResult = 0;
    return;
  }

  if(!*pFlasher->pInput){
    *pResult = 0;
    return;
  }

  if((*pResult == 0) && ((nNow - pFlasher->nTimeOff) > pFlasher->nFlashOffTime)){
    *pResult = 1;
    pFlasher->nTimeOn = nNow;
  }
  if((*pResult == 1) && ((nNow - pFlasher->nTimeOn) > pFlasher->nFlashOnTime)){
    *pResult = 0;
    pFlasher->nTimeOff = nNow;
  }

}
