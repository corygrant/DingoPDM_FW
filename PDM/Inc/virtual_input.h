/*
 * virtual_input.h
 *
 *  Created on: Jan 6, 2021
 *      Author: coryg
 */

#ifndef COMPONENTS_INC_VIRTUAL_INPUT_H_
#define COMPONENTS_INC_VIRTUAL_INPUT_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"
#include "pdm_config.h"

void EvaluateVirtInput(PdmConfig_VirtualInput_t *pIn, uint16_t* pResult);

#endif /* COMPONENTS_INC_VIRTUAL_INPUT_H_ */
