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
bool GetAnyCanInEnable();
bool GetCanInEnable(uint8_t nInput);
bool GetCanInOutput(uint8_t nInput);
uint16_t GetCanInVal(uint8_t nInput);
bool GetAnyVirtInEnable();
bool GetVirtInVal(uint8_t nInput);
bool GetWiperEnable();
bool GetWiperFastOut();
bool GetWiperSlowOut();
WiperState GetWiperState();
WiperSpeed GetWiperSpeed();
bool GetAnyFlasherEnable();
bool GetFlasherVal(uint8_t nFlasher);
bool GetAnyCounterEnable();
uint16_t GetCounterVal(uint8_t nCounter);
bool GetAnyConditionEnable();
bool GetConditionVal(uint8_t nCondition);
bool GetAnyKeypadEnable();
bool GetKeypadEnable(uint8_t nKeypad);
bool GetKeypadButtonVal(uint8_t nKeypad, uint8_t nButton);
uint32_t GetKeypadButtons(uint8_t nKeypad);
uint16_t GetKeypadDialVal(uint8_t nKeypad, uint8_t nDial);