#include <input.h>

void SetInputPull(GPIO_TypeDef  *GPIOx, uint16_t nPin, uint8_t nPull){
  uint32_t position;
  uint32_t ioposition = 0x00U;
  uint32_t iocurrent = 0x00U;
  uint32_t temp = 0x00U;

  /* Configure the port pins */
  for(position = 0U; position < 16; position++)
  {
    /* Get the IO position */
    ioposition = 0x01U << position;
    /* Get the current IO position */
    iocurrent = (uint32_t)(nPin & ioposition);

    if(iocurrent == ioposition)
    {
      temp = GPIOx->PUPDR;
      temp &= ~(GPIO_PUPDR_PUPDR0 << (position * 2U));
      temp |= ((nPull) << (position * 2U));
      GPIOx->PUPDR = temp;
    }
  }
}

void CheckInput(InputVars_t* stInVars, InputMode_t eMode, bool bInvertInput, bool bInput, uint16_t* nOutput, uint16_t nDebounceTime)
{
  bool bInputResult;

  //XOR to flip input
  bInputResult = bInput ^ bInvertInput;

  if(eMode == MODE_MOMENTARY)
  {
    //Check for button change
    //Store trigger time
    if((bInputResult != stInVars->bLastState) && 
       (bInputResult != *nOutput)) //Rising/Falling
    {
        stInVars->nLastTrigTime = HAL_GetTick();
        stInVars->bCheckTime = true;
    }
    
    stInVars->bLastState = bInputResult;

    if((stInVars->bCheckTime && ((HAL_GetTick() - stInVars->nLastTrigTime) > nDebounceTime)) 
      || (!stInVars->bInit))
    {
      stInVars->bCheckTime = false;
      *nOutput = bInputResult;
    }
  }

  if(eMode == MODE_LATCHING)
  {
    //Check for rising trigger
    //Store trigger time
    if((bInputResult != stInVars->bLastState) && 
       (bInputResult == true))
    {
      stInVars->nLastTrigTime = HAL_GetTick();
      stInVars->bCheckTime = true;
    }

    stInVars->bLastState = bInputResult;

    if((stInVars->bCheckTime && ((HAL_GetTick() - stInVars->nLastTrigTime) > nDebounceTime)) 
      || (!stInVars->bInit))
    {
      stInVars->bCheckTime = false;
      *nOutput = !*nOutput;
    }
  }

  stInVars->bInit = true;
}
