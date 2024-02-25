#include "error.h"

void ErrorState(uint8_t nErrorId)
{
  //Handle flashing the LED based on nErrorId
  LedSetSteady(&ErrorLed, true);
  //LedSetCode(&ErrorLed, nErrorId);
}