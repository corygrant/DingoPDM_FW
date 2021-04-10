/*
 * logger.h
 *
 *  Created on: Oct 22, 2020
 *      Author: coryg
 */

#ifndef INC_LOGGER_H_
#define INC_LOGGER_H_
#include "stdio.h"

#define ENABLE_SWO_LOGGING  //Enable SWO logging - turn off to increase performance

typedef enum
{
  TO_USB,
  TO_CAN,
  TO_USB_CAN
} Logger_MsgDest_t;

typedef enum
{
  TX_QUEUE,
  TX_NOW
} Logger_MsgPrio_t;

typedef struct
{
  char msg[20];
  Logger_MsgDest_t eDest;
  Logger_MsgPrio_t ePrio;
} Logger_Msg_t;

typedef struct
{
  uint8_t nQueueReadIndex;
  uint8_t nQueueWriteIndex;
  char sUSBQueue[10][20];
  char sCANQueue[10][20];
} Logger_t;

void logger(char* msg, Logger_MsgDest_t eDest, Logger_MsgPrio_t ePrio);

#endif /* INC_LOGGER_H_ */
