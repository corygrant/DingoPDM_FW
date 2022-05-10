/*
 * pushbutton.c
 *
 *  Created on: Jan 3, 2021
 *      Author: coryg
 */

#include "pushbutton.h"

void CheckPushbutton(PushbuttonConfig_t* pb, PushbuttonMode_t mode, uint16_t nInput, uint16_t* nOutput, uint16_t nDebounceTime)
{
  //=======================================================
  //Debounce and latch logic
  //=======================================================

  if(mode == MODE_MOMENTARY)
  {
    //Check for button change
    //Store trigger time
    if(nInput != pb->nLastState)
    {
      if(    ((nInput == 1) && (*nOutput == 0)) //Rising
          || ((nInput == 0) && (*nOutput == 1))) //Falling
      {
        pb->nLastTrigTime = HAL_GetTick();
        pb->nCheckTime = 1;
      }
    }

    pb->nLastState = nInput;

    if((pb->nCheckTime > 0) && ((HAL_GetTick() - pb->nLastTrigTime) > nDebounceTime))
    {
      pb->nCheckTime = 0;
      *nOutput = nInput;
    }

    //Don't change output
  }

  if(mode == MODE_LATCHING)
  {
    //Check for rising trigger
    //Store trigger time
    if((nInput != pb->nLastState) && (nInput == 1))
    {
      pb->nLastTrigTime = HAL_GetTick();
      pb->nCheckTime = 1;
    }

    pb->nLastState = nInput;

    if((pb->nCheckTime > 0) && ((HAL_GetTick() - pb->nLastTrigTime) > nDebounceTime))
    {
      pb->nCheckTime = 0;
      *nOutput = !*nOutput;
    }
  }
}
