#include "error.h"

void FatalError(PdmFatalError_t eErrorId)
{
  while(1){
    LedSetCode(HAL_GetTick(), &ErrorLed, eErrorId);
  }
}