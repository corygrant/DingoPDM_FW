#pragma once

#define STM32_TEMP_3V3_30C *((uint16_t *)0x1FFF7A2C)
#define STM32_TEMP_3V3_110C *((uint16_t *)0x1FFF7A2E)

#define STM32_VREF_INT_CAL *((uint16_t *)0x1FFF7A2A)

void EnterStopMode();
void RequestBootloader();
void CheckBootloaderRequest(void);