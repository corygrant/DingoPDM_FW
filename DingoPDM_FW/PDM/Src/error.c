#include "error.h"

void FatalError(PdmFatalError_t eErrorId)
{
  //Handle flashing the LED based on nErrorId
  LedSetSteady(&ErrorLed, true);
  //LedSetCode(&ErrorLed, nErrorId);
}