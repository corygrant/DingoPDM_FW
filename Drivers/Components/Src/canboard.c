/*
 * canboard.c
 *
 *  Created on: Jan 1, 2021
 *      Author: coryg
 */

#include "canboard.h"

void CANBoardProcessRxMsg0(volatile CANBoard_RX_t* rx, uint8_t msg[8])
{
  rx->nAnInmV[0] = (msg[1] << 8) + (msg[0]);
  rx->nAnInmV[1] = (msg[3] << 8) + (msg[2]);
  rx->nAnInmV[2] = (msg[5] << 8) + (msg[4]);
  rx->nAnInmV[3] = (msg[7] << 8) + (msg[6]);
}

void CANBoardProcessRxMsg1(volatile CANBoard_RX_t* rx, uint8_t msg[8])
{
  rx->nAnInmV[4] = (msg[1] << 8) + (msg[0]);

  rx->nTempC = (msg[7] << 8) + (msg[6]);

  rx->fTempC = ((float)rx->nTempC / 100.0);
}

void CANBoardProcessRxMsg2(volatile CANBoard_RX_t* rx, uint8_t msg[8])
{
  rx->nRotarySw[0] = msg[0] & 0xF;
  rx->nRotarySw[1] = msg[0] & 0xF0;
  rx->nRotarySw[2] = msg[1] & 0xF;
  rx->nRotarySw[3] = msg[1] & 0xF0;
  rx->nRotarySw[4] = msg[2] & 0xF;

  rx->nDigIn[0] = msg[4] & 0x1;
  rx->nDigIn[1] = (msg[4] & 0x2) >> 1;
  rx->nDigIn[2] = (msg[4] & 0x4) >> 2;
  rx->nDigIn[3] = (msg[4] & 0x8) >> 3;
  rx->nDigIn[4] = (msg[4] & 0x10) >> 4;
  rx->nDigIn[5] = (msg[4] & 0x20) >> 5;
  rx->nDigIn[6] = (msg[4] & 0x40) >> 6;
  rx->nDigIn[7] = (msg[4] & 0x80) >> 7;

  rx->nAnInDig[0] = msg[5] & 0x1;
  rx->nAnInDig[1] = (msg[5] & 0x2) >> 1;
  rx->nAnInDig[2] = (msg[5] & 0x4) >> 2;
  rx->nAnInDig[3] = (msg[5] & 0x8) >> 3;
  rx->nAnInDig[4] = (msg[5] & 0x10) >> 4;

  rx->nDigOut[0] = msg[6] & 0x1;
  rx->nDigOut[1] = (msg[6] & 0x2) >> 1;
  rx->nDigOut[2] = (msg[6] & 0x4) >> 2;
  rx->nDigOut[3] = (msg[6] & 0x8) >> 3;

  rx->nHeartbeat = msg[7];
  rx->nLastHeartbeatTime = HAL_GetTick();
}

void CANBoardCheckConnection(volatile CANBoard_RX_t* rx)
{
  if( (rx->nHeartbeat == rx->nLastHeartbeat) &&
      ((HAL_GetTick() - rx->nLastHeartbeatTime) > (CANBOARD_TX_DELAY * 4)))
  {
    rx->nConnected = 0;
  }
  else
  {
    rx->nConnected = 1;
  }
  rx->nLastHeartbeat = rx->nHeartbeat;
}
