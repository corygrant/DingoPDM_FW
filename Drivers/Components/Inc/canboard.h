/*
 * canboard.h
 *
 *  Created on: Jan 1, 2021
 *      Author: coryg
 */

#ifndef COMPONENTS_INC_CANBOARD_H_
#define COMPONENTS_INC_CANBOARD_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

#define CANBOARD_TX_DELAY 50

typedef struct
{
  uint16_t nBaseId;
  uint16_t nAnInmV[5];
  uint16_t nTempC;
  float fTempC;
  uint8_t nRotarySw[5];
  uint8_t nDigIn[8];
  uint8_t nAnInDig[5];
  uint8_t nDigOut[4];
  uint8_t nHeartbeat;
  uint8_t nLastHeartbeat;
  uint32_t nLastHeartbeatTime;
  uint8_t nConnected;
} CANBoard_RX_t;

typedef struct
{
  uint16_t nBaseId;
  uint8_t nDigOut[4];
} CANBoard_TX_t;
//TODO:Implement CANBoard TX

void CANBoardProcessRxMsg0(volatile CANBoard_RX_t* rx, uint8_t msg[8]);
void CANBoardProcessRxMsg1(volatile CANBoard_RX_t* rx, uint8_t msg[8]);
void CANBoardProcessRxMsg2(volatile CANBoard_RX_t* rx, uint8_t msg[8]);
void CANBoardCheckConnection(volatile CANBoard_RX_t* rx);
#endif /* COMPONENTS_INC_CANBOARD_H_ */
