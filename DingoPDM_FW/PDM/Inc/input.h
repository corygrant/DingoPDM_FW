#ifndef COMPONENTS_INC_PUSHBUTTON_H_
#define COMPONENTS_INC_PUSHBUTTON_H_

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"

#define NO_DEBOUNCE 0

typedef enum
{
  MODE_NUM, //Only used by CAN Inputs
  MODE_MOMENTARY,
  MODE_LATCHING
} InputMode_t;

typedef struct
{
  bool bInit;
  uint32_t nLastTrigTime;
  bool bLastState;
  bool bCheckTime;
} InputVars_t;

void SetInputPull(GPIO_TypeDef  *GPIOx, uint16_t nPin, uint8_t nPull);
void CheckInput(InputVars_t* stInVars, InputMode_t eMode, bool bInvertInput, bool bInput, uint16_t* nOutput, uint16_t nDebounceTime);

#endif /* COMPONENTS_INC_PUSHBUTTON_H_ */
