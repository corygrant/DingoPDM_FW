#pragma once

#include <cstdint>

void InitCan();
void DeInitCan();
void StopCan();
void StartCan();
uint32_t GetLastCanRxTime();