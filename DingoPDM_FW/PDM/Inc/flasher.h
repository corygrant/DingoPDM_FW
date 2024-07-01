#ifndef INC_FLASHER_H_
#define INC_FLASHER_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "pdm_config.h"

void EvaluateFlasher(uint32_t nNow, PdmConfig_Flasher_t* pFlasher, uint16_t* pResult);

#endif /* INC_FLASHER_H_ */
