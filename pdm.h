#pragma once

#include "enums.h"

void CheckBootloaderRequest();
void InitPdm();
PdmState GetPdmState();
float GetBoardTemp();
float GetTotalCurrent();
bool GetInputVal(uint8_t nInput);
uint16_t GetOutputCurrent(uint8_t nOutput);
ProfetState GetOutputState(uint8_t nOutput);
uint8_t GetOutputOcCount(uint8_t nOutput);
bool GetCanInVal(uint8_t nInput);
bool GetVirtInVal(uint8_t nInput);
bool GetWiperFastOut();
bool GetWiperSlowOut();
WiperState GetWiperState();
WiperSpeed GetWiperSpeed();
bool GetFlasherVal(uint8_t nFlasher);