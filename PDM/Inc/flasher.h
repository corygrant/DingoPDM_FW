/*
 * flasher.h
 *
 *  Created on: Jan 6, 2021
 *      Author: coryg
 */

#ifndef INC_FLASHER_H_
#define INC_FLASHER_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"
#include "pdm_config.h"

void EvaluateFlasher(PdmConfig_Flasher_t* pFlasher, uint16_t pResult[PDM_NUM_OUTPUTS]);

#endif /* INC_FLASHER_H_ */
