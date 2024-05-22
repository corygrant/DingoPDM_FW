#ifndef INC_PDM_INPUT_H_
#define INC_PDM_INPUT_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "pdm_config.h"

void EvaluateInput(PdmConfig_Input_t *pIn, uint16_t* pResult);

#endif /* INC_PDM_INPUT_H_ */
