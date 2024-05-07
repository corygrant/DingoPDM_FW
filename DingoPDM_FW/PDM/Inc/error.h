#ifndef INC_ERROR_H_
#define INC_ERROR_H_

#include "stdint.h"
#include "stdbool.h"
#include "status_led.h"

typedef enum{
  PDM_ERROR_NONE = 0,
  PDM_ERROR_IWDG,
  PDM_ERROR_MSG_QUEUE,
  PDM_ERROR_TASK,
  PDM_ERROR_CONFIG,
  PDM_ERROR_FRAM,
  PDM_ERROR_ADC,
  PDM_ERROR_TEMP_SENSOR,
  PDM_ERROR_USB,
  PDM_ERROR_CAN,
  PDM_ERROR_CRC,
  PDM_ERROR_I2C,
  PDM_ERROR_RCC
} PdmErrorId_t;

void ErrorState(uint8_t nErrorId);

#endif /* INC_ERROR_H_ */