/*
 * profet.h
 *
 *  Created on: Nov 9, 2020
 *      Author: coryg
 */

#ifndef COMPONENTS_INC_PROFET_H_
#define COMPONENTS_INC_PROFET_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

typedef enum{
  BTS7002_1EPP,
  BTS7008_2EPA_CH1,
  BTS7008_2EPA_CH2
} ProfetModelTypeDef;

typedef enum {
  OFF = 0,
  ON,
  IN_RUSH,
  SHORT_CIRCUIT,
  OVERCURRENT,
  FAULT,
  SUSPENDED,
  TURNING_OFF,
  TURNING_ON,
  IN_RUSHING,
  SHORT_CIRCUITING,
  OVERCURRENTING,
  FAULTING,
  SUSPENDING
} ProfetStateTypeDef;

typedef struct {
  ProfetModelTypeDef eModel;
  volatile ProfetStateTypeDef eState;
  volatile ProfetStateTypeDef eReqState;
  volatile char cState;
  uint16_t nNum;
  uint16_t *nIN_Port;
  uint16_t nIN_Pin;
  volatile uint16_t nIS_Avg;
  volatile uint32_t nIS_Sum;
  uint16_t nIL_Limit;
  uint16_t nIL_InRush_Limit;
  volatile uint16_t nIL_InRush_Time;
  volatile uint32_t nIL_On_Time;
  volatile uint16_t nIL;
  volatile uint16_t nIS;
  volatile uint16_t nValStore;
  volatile uint16_t nOvercurrentCnt;
  volatile uint16_t nOC_ResetTime;
  volatile uint32_t nOC_TriggerTime;
  volatile uint8_t nOC_ResetCount;
  volatile uint8_t nOC_ResetLimit;
  volatile uint8_t nOC_Detected;
  float fKilis;

} ProfetTypeDef;

typedef struct {
  uint32_t nStartTime;
  uint32_t nDelayTime;
  //bool bActive;
  //bool bDone;
  //bool bLastVal;
} ProfetDelayTypeDef;

void Profet_SM(volatile ProfetTypeDef *profet);
void Profet_UpdateIS(volatile ProfetTypeDef *profet, uint16_t newVal);
uint32_t GetTripTime(ProfetModelTypeDef eModel, uint16_t nIL, uint16_t nMaxIL);
#endif /* COMPONENTS_INC_PROFET_H_ */
