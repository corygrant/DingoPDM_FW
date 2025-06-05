#pragma once

#include "enums.h"
#include "config.h"

class CanInput;
class VirtualInput;
class Profet;
class Wiper;
class Starter;
class Flasher;
class Counter;
class Condition;
class Keypad;

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
extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];
extern PdmState eState;
extern float fTempSensor;
extern float fBattVolt;
extern bool bSleepRequest;

void CheckBootloaderRequest();
void InitPdm();