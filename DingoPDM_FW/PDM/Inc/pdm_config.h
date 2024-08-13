#ifndef INC_PDM_CONFIG_H_
#define INC_PDM_CONFIG_H_

#include "input.h"
#include "msg_queue.h"
#include "stdint.h"
#include "stdbool.h"
#include "cmsis_os.h"
#include "string.h"
#include "mb85rc.h"
#include "profet.h"
#include "wipers.h"

#define PDM_MAJOR_VERSION 0
#define PDM_MINOR_VERSION 3
#define PDM_BUILD 4

#define PDM_NUM_OUTPUTS 8
#define PDM_NUM_INPUTS 2
#define PDM_NUM_VIRT_INPUTS 16
#define PDM_NUM_CAN_INPUTS 32
#define PDM_NUM_FLASHERS 4
#define PDM_NUM_WIPER_INTER_DELAYS 6
#define PDM_NUM_WIPER_SPEED_MAP 8

#define PDM_VAR_MAP_SIZE 66

#define CAN_TX_SETTING_ID_OFFSET 30
#define CAN_TX_MSG_ID_OFFSET 31

typedef enum{
  OPER_EQUAL,
  OPER_GREATER_THAN,
  OPER_LESS_THAN,
  OPER_BITWISE_AND,
  OPER_BITWISE_NAND
} PdmConfig_Operator_t;



typedef enum{
  COND_AND,
  COND_OR,
  COND_NOR
} PdmConfig_Condition_t;

typedef enum{
  NOPULL,
  PULLUP,
  PULLDOWN
} PdmConfig_InputPull;

typedef struct{
  uint8_t nVersion;
  uint8_t nCanEnabled;
  uint8_t nCanSpeed;
} PdmConfig_DeviceConfig_t;

typedef struct{
  uint8_t nEnabled;
  bool* pInput;
  InputMode_t eMode;
  InputVars_t stInVars;
  bool bInvert;
  uint16_t nDebounceTime;
  PdmConfig_InputPull ePull;
  GPIO_TypeDef  *GPIOx;
  uint16_t nPin;
} PdmConfig_Input_t;

typedef struct{
  uint8_t nEnabled;
  uint8_t nNot0;
  uint8_t nVar0;
  uint16_t* pVar0;
  PdmConfig_Condition_t eCond0;
  uint8_t nNot1;
  uint8_t nVar1;
  uint16_t* pVar1;
  PdmConfig_Condition_t eCond1;
  uint8_t nNot2;
  uint8_t nVar2;
  uint16_t* pVar2;
  InputMode_t eMode;
  InputVars_t stInVars;
} PdmConfig_VirtualInput_t;

typedef struct{
  uint8_t nEnabled;
  uint8_t nInput;
  uint16_t* pInput;
  uint16_t nCurrentLimit;
  uint16_t nInrushLimit;
  uint16_t nInrushTime;
  ProfetResetMode_t eResetMode;
  uint16_t nResetTime;
  uint8_t nResetLimit;
} PdmConfig_Output_t;

typedef struct{
  uint8_t nEnabled;
  uint8_t nMode;
  uint8_t nSlowInput;   //MODE_DIG_IN
  uint8_t nFastInput;   //MODE_DIG_IN
  uint8_t nInterInput;  //MODE_DIG_IN
  uint8_t nOnInput;     //MODE_MIX_IN
  uint8_t nSpeedInput;  //MODE_INT_IN and MODE_MIX_IN
  uint8_t nParkInput;
  uint8_t nParkStopLevel;
  uint8_t nSwipeInput;
  uint8_t nWashInput;
  uint8_t nWashWipeCycles;
  uint8_t nSpeedMap[PDM_NUM_WIPER_SPEED_MAP];
  uint16_t nIntermitTime[PDM_NUM_WIPER_INTER_DELAYS];
} PdmConfig_Wiper_t;

typedef struct{
  uint8_t nEnabled;
  uint8_t nInput;
  uint16_t* pInput;
  uint16_t nFlashOnTime;
  uint16_t nFlashOffTime;
  uint8_t nSingleCycle;
  uint32_t nTimeOff;
  uint32_t nTimeOn;
} PdmConfig_Flasher_t;

typedef struct{
  uint8_t nEnabled;
  uint8_t nInput;
  uint16_t* pInput;
  uint8_t nDisableOut[PDM_NUM_OUTPUTS];
} PdmConfig_Starter_t;

typedef struct{
  uint8_t nEnabled;
  uint16_t nId;
  uint16_t nLowByte;
  uint16_t nHighByte;
  PdmConfig_Operator_t eOperator;
  uint16_t nOnVal;
  InputMode_t eMode;
  InputVars_t stInVars;
} PdmConfig_CanInput_t;

typedef struct{
  uint8_t nEnabled;
  uint16_t nBaseId;
  uint16_t nUpdateTime;
} PdmConfig_CanOutput_t;

typedef struct{
  PdmConfig_DeviceConfig_t stDevConfig;
  PdmConfig_Input_t stInput[PDM_NUM_INPUTS];
  PdmConfig_VirtualInput_t stVirtualInput[PDM_NUM_VIRT_INPUTS];
  PdmConfig_Output_t stOutput[PDM_NUM_OUTPUTS];
  PdmConfig_Wiper_t stWiper;
  PdmConfig_Flasher_t stFlasher[PDM_NUM_FLASHERS];
  PdmConfig_Starter_t stStarter;
  PdmConfig_CanInput_t stCanInput[PDM_NUM_CAN_INPUTS];
  PdmConfig_CanOutput_t stCanOutput;
} PdmConfig_t;

uint8_t PdmConfig_Check(I2C_HandleTypeDef* hi2c, uint8_t nAddr, PdmConfig_t* pConfig);
uint8_t PdmConfig_Read(I2C_HandleTypeDef* hi2c, uint8_t nAddr, PdmConfig_t* pConfig);
uint8_t PdmConfig_Write(I2C_HandleTypeDef* hi2c, uint8_t nAddr, PdmConfig_t* pConfig);
uint8_t PdmConfig_Set(PdmConfig_t* pConfig, uint16_t* pVariableMap[PDM_VAR_MAP_SIZE], volatile ProfetTypeDef profet[PDM_NUM_OUTPUTS], Wiper_t* pWiper, MsgQueueRx_t* stMsgRx, osMessageQueueId_t* qMsgQueueTx);
void PdmConfig_SetDefault(PdmConfig_t* pConfig);

#endif /* INC_PDM_CONFIG_H_ */
