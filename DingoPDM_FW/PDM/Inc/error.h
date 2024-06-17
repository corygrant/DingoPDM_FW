#ifndef INC_ERROR_H_
#define INC_ERROR_H_

#include "stdint.h"
#include "stdbool.h"
#include "status_led.h"
#include "stm32f4xx_hal.h"

typedef enum{
  FATAL_ERROR_NONE = 0,
  FATAL_ERROR_IWDG,
  FATAL_ERROR_MSG_QUEUE,
  FATAL_ERROR_TASK,
  FATAL_ERROR_CONFIG,
  FATAL_ERROR_FRAM,
  FATAL_ERROR_ADC,
  FATAL_ERROR_TEMP_SENSOR,
  FATAL_ERROR_USB,
  FATAL_ERROR_CAN,
  FATAL_ERROR_CRC,
  FATAL_ERROR_I2C,
  FATAL_ERROR_RCC,
  FATAL_ERROR_TEMP
} PdmFatalError_t;

void FatalError(PdmFatalError_t eErrorId);

#endif /* INC_ERROR_H_ */