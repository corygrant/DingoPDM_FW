#ifndef INC_CAN_INPUT_H_
#define INC_CAN_INPUT_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "pdm_config.h"

typedef struct{
  bool bRxOk;
  uint32_t nLastRxTime;
  uint32_t nRxMaxTime;
} CANInput_Rx_t;

uint8_t EvaluateCANInput(CAN_RxHeaderTypeDef* stRxHeader, uint8_t nRxData[8], PdmConfig_CanInput_t *stIn, uint16_t* nResult);

#endif /* INC_CAN_INPUT_H_ */
