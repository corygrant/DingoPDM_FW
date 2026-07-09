#pragma once

#include "enums.h"
#include "config.h"
#include "status.h"

class CanInput;
class CanOutputs;
class VirtualInput;
class Flasher;
class Counter;
class Condition;

#if NUM_DIG_INPUTS > 0
class Digital_Input;
#endif
#if NUM_OUTPUTS > 0
class Profet;
#endif
#if HAS_WIPERS
class Wiper;
#endif
#if HAS_STARTER_DISABLE
class Starter;
#endif
#if NUM_KEYPADS > 0
class Keypad;
#endif
#if NUM_DIG_OUTPUTS > 0
class Digital_Output;
#endif
#if NUM_ANALOG_INPUTS > 0
class Analog_Input;
#endif


extern CanInput canIn[NUM_CAN_INPUTS];
extern CanOutputs canOutputs;
extern VirtualInput virtIn[NUM_VIRT_INPUTS];
extern Flasher flasher[NUM_FLASHERS];
extern Counter counter[NUM_COUNTERS];
extern Condition condition[NUM_CONDITIONS];

#if NUM_DIG_INPUTS > 0
extern Digital_Input digIn[NUM_DIG_INPUTS];
#endif
#if NUM_OUTPUTS > 0
extern Profet pf[NUM_OUTPUTS];
#endif
#if HAS_WIPERS
extern Wiper wiper;
#endif
#if HAS_STARTER_DISABLE
extern Starter starter;
#endif
#if NUM_KEYPADS > 0
extern Keypad keypad[NUM_KEYPADS];
#endif
#if NUM_DIG_OUTPUTS > 0
extern Digital_Output digOut[NUM_DIG_OUTPUTS];
#endif
#if NUM_ANALOG_INPUTS > 0
extern Analog_Input analogIn[NUM_ANALOG_INPUTS];
#endif

extern DeviceConfig stConfig;
extern DeviceConfig stConfigTemp; // Used for staging new config before applying
extern float *pVarMap[VAR_MAP_SIZE];
extern DeviceState eState;
extern bool bDeviceOverTemp;
extern bool bDeviceCriticalTemp;

#if HAS_EXT_TEMP_SENSOR
extern float fTempSensor;
#endif
#if HAS_BATT_VOLT_SENSE
extern float fBattVolt;
#endif
#if CAN_SLEEP
extern bool bSleepRequest;
#endif

#if HAS_USB
void CheckBootloaderRequest();
#endif

void InitDevice();