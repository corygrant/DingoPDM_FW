#pragma once

#include <cstdint>
#include "port.h"

void InitAdc();
void DeInitAdc();
uint16_t GetAdcRaw(AnalogChannel channel);
float GetBattVolt();
float GetTemperature();
float GetVDDA();