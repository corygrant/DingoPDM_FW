#pragma once

#include <cstdint>
#include "enums.h"

PdmState GetPdmState();
float GetBoardTemp();
float GetTotalCurrent();

bool GetAnyOvercurrent();
bool GetAnyFault();

bool GetInputVal(uint8_t nInput);

float GetOutputCurrent(uint8_t nOutput);
ProfetState GetOutputState(uint8_t nOutput);
uint8_t GetOutputOcCount(uint8_t nOutput);
uint8_t GetOutputDC(uint8_t nOutput);

bool GetAnyPwmEnable();
bool GetAnyCanInEnable();
bool GetAnyVirtInEnable();
bool GetWiperEnable();
bool GetAnyFlasherEnable();
bool GetAnyCounterEnable();
bool GetAnyConditionEnable();
bool GetAnyKeypadEnable();

bool GetCanInEnable(uint8_t nInput);
bool GetCanInOutput(uint8_t nInput);
float GetCanInVal(uint8_t nInput);
uint32_t GetCanInOutputs();

bool GetVirtInVal(uint8_t nInput);
uint32_t GetVirtIns();

bool GetWiperFastOut();
bool GetWiperSlowOut();
WiperState GetWiperState();
WiperSpeed GetWiperSpeed();

bool GetFlasherVal(uint8_t nFlasher);

float GetCounterVal(uint8_t nCounter);

uint32_t GetConditions();

bool GetKeypadEnable(uint8_t nKeypad);
uint32_t GetKeypadButtons(uint8_t nKeypad);
float GetKeypadDialVal(uint8_t nKeypad, uint8_t nDial);