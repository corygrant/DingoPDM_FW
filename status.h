#pragma once

#include <cstdint>
#include "enums.h"

// System status query functions extracted from pdm.cpp

// Core system status
PdmState GetPdmState();
float GetBoardTemp();
float GetTotalCurrent();

// Utility functions for internal use
bool GetAnyOvercurrent();
bool GetAnyFault();

// Input queries
bool GetInputVal(uint8_t nInput);

// Output queries
uint16_t GetOutputCurrent(uint8_t nOutput);
ProfetState GetOutputState(uint8_t nOutput);
uint8_t GetOutputOcCount(uint8_t nOutput);
uint8_t GetOutputDC(uint8_t nOutput);

// Feature enable status
bool GetAnyPwmEnable();
bool GetAnyCanInEnable();
bool GetAnyVirtInEnable();
bool GetWiperEnable();
bool GetAnyFlasherEnable();
bool GetAnyCounterEnable();
bool GetAnyConditionEnable();
bool GetAnyKeypadEnable();

// CAN input queries
bool GetCanInEnable(uint8_t nInput);
bool GetCanInOutput(uint8_t nInput);
uint16_t GetCanInVal(uint8_t nInput);
uint32_t GetCanInOutputs();

// Virtual input queries
bool GetVirtInVal(uint8_t nInput);
uint32_t GetVirtIns();

// Wiper queries
bool GetWiperFastOut();
bool GetWiperSlowOut();
WiperState GetWiperState();
WiperSpeed GetWiperSpeed();

// Flasher queries
bool GetFlasherVal(uint8_t nFlasher);

// Counter queries
uint16_t GetCounterVal(uint8_t nCounter);

// Condition queries
uint32_t GetConditions();

// Keypad queries
bool GetKeypadEnable(uint8_t nKeypad);
uint32_t GetKeypadButtons(uint8_t nKeypad);
uint16_t GetKeypadDialVal(uint8_t nKeypad, uint8_t nDial);