#pragma once

#include <cstdint>
#include "enums.h"

void InitCan(CanBitrate bitrate);
void DeInitCan();
void StopCan();
void StartCan(CanBitrate bitrate);
uint32_t GetLastCanRxTime();