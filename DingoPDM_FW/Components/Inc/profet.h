#ifndef COMPONENTS_INC_PROFET_H_
#define COMPONENTS_INC_PROFET_H_

#include "stdint.h"
#include "stdbool.h"
#include "stm32f4xx_hal.h"
#include "msg_queue.h"
#include "cmsis_os.h"

typedef enum{
  BTS7002_1EPP,
  BTS7008_2EPA_CH1,
  BTS7008_2EPA_CH2,
  BTS724_CH1,
  BTS724_CH2,
  BTS724_CH3,
  BTS724_CH4
} ProfetModelTypeDef;

typedef enum {
  OFF = 0,
  ON,
  OVERCURRENT,
  FAULT
} ProfetStateTypeDef;

typedef enum{
  RESET_NONE,
  RESET_COUNT,
  RESET_ENDLESS
} ProfetResetMode_t;

typedef struct {
  ProfetModelTypeDef eModel;
  volatile ProfetStateTypeDef eState;
  volatile ProfetStateTypeDef eLastState;
  volatile ProfetStateTypeDef eReqState;
  volatile ProfetResetMode_t eResetMode;
  volatile char cState;

  bool bEnabled;
  uint16_t nNum;

  GPIO_TypeDef* nIN_Port;
  uint16_t nIN_Pin;
  GPIO_TypeDef* nDEN_Port;
  uint16_t nDEN_Pin;

  uint16_t nIL_Limit; //Current limit (amps)

  bool bInRushActive; //Currently in inrush
  uint16_t nIL_InRushLimit; //Current limit during inrush
  volatile uint16_t nIL_InRushTime; //Inrush time limit
  volatile uint32_t nInRushOnTime; //ms

  volatile uint16_t nIL; //Scaled current value (amps)
  volatile uint16_t nIS; //Raw analog current value

  volatile uint16_t nOC_Count; //Number of overcurrents
  volatile uint16_t nOC_ResetTime; //Time after overcurrent before reset
  volatile uint32_t nOC_TriggerTime; //Time of overcurrent
  volatile uint8_t nOC_ResetLimit; //Limit of number of overcurrent resets

  float fKILIS;

} ProfetTypeDef;

void Profet_SM(volatile ProfetTypeDef *profet, bool bOutputsOk);
void Profet_UpdateIS(volatile ProfetTypeDef *profet, uint16_t newVal, volatile float fVDDA);
#endif /* COMPONENTS_INC_PROFET_H_ */
