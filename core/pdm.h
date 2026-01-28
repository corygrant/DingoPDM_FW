#pragma once

#include "enums.h"
#include "config.h"

class Digital;
class CanInput;
class VirtualInput;
class Profet;
class Wiper;
class Starter;
class Flasher;
class Counter;
class Condition;
class Keypad;

extern Digital in[PDM_NUM_INPUTS];
extern CanInput canIn[PDM_NUM_CAN_INPUTS];
extern VirtualInput virtIn[PDM_NUM_VIRT_INPUTS];
extern Profet pf[PDM_NUM_OUTPUTS];
extern Wiper wiper;
extern Starter starter;
extern Flasher flasher[PDM_NUM_FLASHERS];
extern Counter counter[PDM_NUM_COUNTERS];
extern Condition condition[PDM_NUM_CONDITIONS];
extern Keypad keypad[PDM_NUM_KEYPADS];

extern PdmConfig stConfig;
extern float *pVarMap[PDM_VAR_MAP_SIZE];
extern PdmState eState;
extern float fTempSensor;
extern float fBattVolt;
extern bool bSleepRequest;

void CheckBootloaderRequest();
void InitPdm();

PdmState GetPdmState();
float GetBoardTemp();
float GetTotalCurrent();
bool GetInputVal(uint8_t nInput);
float GetOutputCurrent(uint8_t nOutput);
ProfetState GetOutputState(uint8_t nOutput);
uint8_t GetOutputOcCount(uint8_t nOutput);
uint8_t GetOutputDC(uint8_t nOutput);
bool GetAnyPwmEnable();
bool GetAnyCanInEnable();
bool GetCanInEnable(uint8_t nInput);
bool GetCanInOutput(uint8_t nInput);
float GetCanInVal(uint8_t nInput);
uint32_t GetCanInOutputs();
bool GetAnyVirtInEnable();
uint32_t GetVirtIns();
bool GetVirtInVal(uint8_t nInput);
bool GetWiperEnable();
bool GetWiperFastOut();
bool GetWiperSlowOut();
WiperState GetWiperState();
WiperSpeed GetWiperSpeed();
bool GetAnyFlasherEnable();
bool GetFlasherVal(uint8_t nFlasher);
bool GetAnyCounterEnable();
float GetCounterVal(uint8_t nCounter);
bool GetAnyConditionEnable();
uint32_t GetConditions();
bool GetConditionVal(uint8_t nCondition);
bool GetAnyKeypadEnable();
bool GetKeypadEnable(uint8_t nKeypad);
uint32_t GetKeypadButtons(uint8_t nKeypad);
float GetKeypadDialVal(uint8_t nKeypad, uint8_t nDial);