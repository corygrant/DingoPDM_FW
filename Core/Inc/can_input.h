/*
 * can_input.h
 *
 *  Created on: Jan 5, 2021
 *      Author: coryg
 */

#ifndef INC_CAN_INPUT_H_
#define INC_CAN_INPUT_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"
#include "pdm_config.h"

uint8_t EvaluateCANInput(CAN_RxHeaderTypeDef* stRxHeader, uint8_t nRxData[8], PdmConfig_CanInput_t *in, uint16_t* nResult);

#endif /* INC_CAN_INPUT_H_ */
