#include "pdm_input.h"

void EvaluateInput(PdmConfig_Input_t *pIn, uint16_t* pResult)
{
  if(!pIn->nEnabled)
    return;

  CheckInput(&pIn->stInVars, pIn->eMode, pIn->bInvert, *pIn->pInput, pResult, pIn->nDebounceTime);
}
