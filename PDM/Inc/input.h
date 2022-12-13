/*
 * pushbutton.h
 *
 *  Created on: Jan 3, 2021
 *      Author: coryg
 */

#ifndef COMPONENTS_INC_PUSHBUTTON_H_
#define COMPONENTS_INC_PUSHBUTTON_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

#define NO_DEBOUNCE 0

typedef enum
{
  MODE_NUM,
  MODE_MOMENTARY,
  MODE_LATCHING
} PushbuttonMode_t;

typedef struct
{
  //PdmConfig_SwitchMode_t eButtonMode;
  //uint8_t* nInput;
  //uint8_t nOutput;
  uint8_t nLastState;
  uint32_t nLastTrigTime;
  //uint16_t nDebounceTime;
  uint8_t nCheckTime;
} PushbuttonConfig_t;

void CheckInput(PushbuttonConfig_t* pb, PushbuttonMode_t mode, uint16_t nInput, uint16_t* nOutput, uint16_t nDebounceTime);

#endif /* COMPONENTS_INC_PUSHBUTTON_H_ */
