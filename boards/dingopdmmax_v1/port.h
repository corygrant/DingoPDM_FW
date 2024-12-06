#pragma once

#include "hal.h"

#define PDM_TYPE 1 //0 = PDM, 1 = PDM-MAX 

#define STM32_TEMP_3V3_30C  *((uint16_t*)0x1FFFF7B8)
#define STM32_TEMP_3V3_110C *((uint16_t*)0x1FFFF7C2)

const CANConfig& GetCanConfig();