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
uint8_t GetOutputDC(uint8_t nOutput);
bool GetCanInOutput(uint8_t nInput);
uint16_t GetCanInVal(uint8_t nInput);
bool GetVirtInVal(uint8_t nInput);
bool GetWiperFastOut();
bool GetWiperSlowOut();
WiperState GetWiperState();
WiperSpeed GetWiperSpeed();
bool GetFlasherVal(uint8_t nFlasher);
uint16_t GetCounterVal(uint8_t nCounter);
bool GetConditionVal(uint8_t nCondition);