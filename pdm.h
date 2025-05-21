#pragma once

#include "enums.h"

void CheckBootloaderRequest();
void InitPdm();

uint16_t GetVarMap(VarMap eVar);
PdmState GetPdmState();
float GetBoardTemp();
float GetTotalCurrent();
uint16_t GetOutputCurrent(uint8_t nOutput);
ProfetState GetOutputState(uint8_t nOutput);
uint8_t GetOutputOcCount(uint8_t nOutput);
uint8_t GetOutputDC(uint8_t nOutput);
bool GetAnyCanInEnable();
bool GetCanInEnable(uint8_t nInput);
uint32_t GetCanInOutputs();
bool GetAnyVirtInEnable();
uint32_t GetVirtIns();
bool GetWiperEnable();
WiperState GetWiperState();
WiperSpeed GetWiperSpeed();
bool GetAnyFlasherEnable();
bool GetAnyCounterEnable();
bool GetAnyConditionEnable();
uint32_t GetConditions();
bool GetAnyKeypadEnable();
bool GetKeypadEnable(uint8_t nKeypad);
uint32_t GetKeypadButtons(uint8_t nKeypad);