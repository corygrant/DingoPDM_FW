/*
 * profet.c
 *
 *  Created on: Nov 9, 2020
 *      Author: coryg
 */

#include "profet.h"

uint32_t GetTripTime(ProfetModelTypeDef eModel, uint16_t nIL, uint16_t nMaxIL);

void Profet_SM(volatile ProfetTypeDef *profet) {

  switch (profet->eState) {

  case OFF:
    profet->cState = 'O';

    //Check for turn on
    if (profet->eReqState == ON) {
      HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_SET);
      profet->eState = ON;
    }
    break;

  case ON:
    profet->cState = '|';

    //Check for fault (device overcurrent/overtemp/short)
    //IL will be very high
    //TODO: Calculate value from datasheet
    if (profet->nIS_Avg > 30000) {
      profet->eState = FAULT;
    }

    //Check for turn off
    if (profet->eReqState == OFF) {
      HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_RESET);
      profet->eState = OFF;
    }

    if ((profet->nIL > profet->nIL_Limit) && (profet->nOC_Detected == 0)){
      profet->nIL_On_Time = HAL_GetTick();
      profet->nOC_Detected = 1;
    }

    if ((profet->nIL < profet->nIL_Limit) && (profet->nOC_Detected > 0)){
      profet->nOC_Detected = 0;
    }

    if(profet->nOC_Detected > 0){
      //if((HAL_GetTick() - profet->nIL_On_Time) > GetTripTime(profet->eModel, profet->nIL, profet->nIL_Limit)){
        profet->nValStore = profet->nIL;
        HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_RESET);
        profet->nOC_TriggerTime = HAL_GetTick();
        profet->nOC_ResetCount++;
        profet->eState = OVERCURRENT;
      //}
    }
    break;

  case OVERCURRENT:
    profet->cState = 'C';
    HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_RESET);
    if(profet->nOC_ResetCount > profet->nOC_ResetLimit){
      HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_RESET);
      profet->eState = SUSPENDED;
    }

    //Check for turn off
    if (profet->eReqState == OFF) {
      profet->nOC_ResetCount = 0;
      HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_RESET);
      profet->eState = OFF;
    }
    break;

  case FAULT:
    profet->cState = 'F';
    HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_RESET);
    //TODO: Reset from fault requires DEN cycle
    break;

  case SUSPENDED:
    profet->cState = 'X';
    //TODO: replace with a reset
    if (profet->eReqState == OFF){
      profet->nOC_ResetCount = 0;
      HAL_GPIO_WritePin(profet->nIN_Port, profet->nIN_Pin, GPIO_PIN_RESET);
      profet->eState = OFF;
    }
    break;

  }
}

void Profet_UpdateIS(volatile ProfetTypeDef *profet, uint16_t newVal)
{
  //Moving average without array or dividing
  //Store the new val, incase we need a non-filtered val elsewhere
  profet->nIS = newVal;
  //Add new value to old sum
  profet->nIS_Sum += profet->nIS;
  //Shift sum by 1 which is equal to dividing by 2
  profet->nIS_Avg = profet->nIS_Sum >> 1;
  //Remove the average from the sum, otherwise sum always goes up never down
  profet->nIS_Sum -= profet->nIS_Avg;

  //Convert IS to IL (actual current)
  profet->nIL = (uint16_t)(((float)profet->nIS_Avg * profet->fKilis) / 10.0);

  //Ignore current readings below low threshold
  if((profet->eModel == BTS7002_1EPP) && (profet->nIL < 0.3)){
    profet->nIL = 0.0;
  }

  if(((profet->eModel == BTS7008_2EPA_CH1) || (profet->eModel == BTS7008_2EPA_CH1))
      && (profet->nIL < 0.1)){
      profet->nIL = 0.0;
  }
}
