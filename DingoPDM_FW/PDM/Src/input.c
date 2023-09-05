/*
 * input.c
 *
 *  Created on: Jan 3, 2021
 *      Author: coryg
 *
 *      -Evaluate input debounce, mode and inversion
 */

#include <input.h>

void CheckInput(InputVars_t* stInVars, InputMode_t eMode, bool bInvertInput, bool bInput, uint16_t* nOutput, uint16_t nDebounceTime)
{
  bool bInputResult;

  //XOR to flip input
  bInputResult = bInput ^ bInvertInput;

  if(eMode == MODE_MOMENTARY)
  {
    //Check for button change
    //Store trigger time
    if(bInputResult != stInVars->bLastState)
    {
      if(bInputResult != *nOutput) //Rising/Falling
      {
        stInVars->nLastTrigTime = HAL_GetTick();
        stInVars->bCheckTime = true;
      }
    }

    stInVars->bLastState = bInputResult;

    if(stInVars->bCheckTime && ((HAL_GetTick() - stInVars->nLastTrigTime) > nDebounceTime))
    {
      stInVars->bCheckTime = false;
      *nOutput = bInputResult;
    }

    //Don't change output
  }

  if(eMode == MODE_LATCHING)
  {
    //Check for rising trigger
    //Store trigger time
    if((bInputResult != stInVars->bLastState) && (bInputResult == true))
    {
      stInVars->nLastTrigTime = HAL_GetTick();
      stInVars->bCheckTime = true;
    }

    stInVars->bLastState = bInputResult;

    if(stInVars->bCheckTime && ((HAL_GetTick() - stInVars->nLastTrigTime) > nDebounceTime))
    {
      stInVars->bCheckTime = false;
      *nOutput = !*nOutput;
    }
  }
}
