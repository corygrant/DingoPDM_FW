#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"
#include "hardware/mb85rc.h"

struct Config_DeviceConfig{
  CanBitrate eCanSpeed;
};

struct Config_Input{
  bool bEnabled;
  InputMode eMode;
  bool bInvert;
  uint16_t nDebounceTime;
  InputPull ePull;
};

struct Config_VirtualInput{
  bool bEnabled;
  bool bNot0;
  uint8_t nVar0;
  Condition eCond0;
  bool bNot1;
  uint8_t nVar1;
  Condition eCond1;
  bool bNot2;
  uint8_t nVar2;
  InputMode eMode;
};

struct Config_Output{
  bool bEnabled;
  uint8_t nInput;
  uint16_t nCurrentLimit;
  uint16_t nInrushLimit;
  uint16_t nInrushTime;
  ProfetResetMode eResetMode;
  uint16_t nResetTime;
  uint8_t nResetLimit;
};

struct Config_Wiper{
  bool bEnabled;
  WiperMode eMode;
  uint8_t nSlowInput;   //WiperMode_DigIn
  uint8_t nFastInput;   //WiperMode_DigIn
  uint8_t nInterInput;  //WiperMode_DigIn
  uint8_t nOnInput;     //WiperMode_MixIn
  uint8_t nSpeedInput;  //WiperMode_IntIn and WiperMode_MixIn
  uint8_t nParkInput;
  bool bParkStopLevel;
  uint8_t nSwipeInput;
  uint8_t nWashInput;
  uint8_t nWashWipeCycles;
  WiperSpeed eSpeedMap[PDM_NUM_WIPER_SPEED_MAP];
  uint16_t nIntermitTime[PDM_NUM_WIPER_INTER_DELAYS];
};

struct Config_Flasher{
  bool bEnabled;
  uint8_t nInput;
  uint16_t nFlashOnTime;
  uint16_t nFlashOffTime;
  bool bSingleCycle;
};

struct Config_Starter{
  bool bEnabled;
  uint8_t nInput;
  bool bDisableOut[PDM_NUM_OUTPUTS];
};

struct Config_CanInput{
  bool bEnabled;
  bool bTimeoutEnabled;
  uint16_t nTimeout; //ms
  uint8_t nIDE; //0=STD, 1=EXT
  uint32_t nSID:11;
  uint32_t nEID:29;
  uint16_t nDLC;
  uint16_t nStartingByte;
  Operator eOperator;
  uint16_t nOnVal;
  InputMode eMode;
};

struct Config_CanOutput{
  bool bEnabled;
  uint16_t nBaseId;
  uint16_t nUpdateTime;
};

struct PdmConfig{
  Config_DeviceConfig stDevConfig;
  Config_Input stInput[PDM_NUM_INPUTS];
  Config_VirtualInput stVirtualInput[PDM_NUM_VIRT_INPUTS];
  Config_Output stOutput[PDM_NUM_OUTPUTS];
  Config_Wiper stWiper;
  Config_Flasher stFlasher[PDM_NUM_FLASHERS];
  Config_Starter stStarter;
  Config_CanInput stCanInput[PDM_NUM_CAN_INPUTS];
  Config_CanOutput stCanOutput;
};

extern PdmConfig stConfig;

void InitConfig();
bool WriteConfig();