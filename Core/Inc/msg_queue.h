/*
 * msg_queue.h
 *
 *  Created on: Feb 20, 2021
 *      Author: coryg
 */

#ifndef INC_MSG_QUEUE_H_
#define INC_MSG_QUEUE_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

typedef enum
{
  CAN_RX,
  USB_RX
} MsgQueueRxSrc_t;

typedef enum
{
  MSG_RX_BURN_SETTINGS = 'B',
  MSG_RX_SET_MODE = 'M',
  MSG_RX_FORCE_OUTPUTS = 'Q',
  MSG_RX_SET_REPORTING = 'R',
  MSG_RX_SET_CAN = 'C',
  MSG_RX_SET_LOGGING = 'L',
  MSG_RX_SET_INPUTS = 'I',
  MSG_RX_SET_OUTPUTS = 'O',
  MSG_RX_SET_VIRTUAL_INPUTS = 'U',
  MSG_RX_SET_WIPER = 'W',
  MSG_RX_SET_WIPER_SPEED = 'P',
  MSG_RX_SET_WIPER_DELAYS = 'Y',
  MSG_RX_SET_FLASHER = 'H',
  MSG_RX_SET_STARTER = 'D',
  MSG_RX_SET_CAN_INPUTS = 'N',
  MSG_RX_SET_TIME = 'T',
  MSG_RX_GET_VERSION = 'V'

} MsgQueueRxCmd_t;

typedef enum
{
  MSG_TX_BURN_SETTINGS = 'b',
  MSG_TX_SET_MODE = 'm',
  MSG_TX_FORCE_OUTPUTS = 'q',
  MSG_TX_SET_REPORTING = 'r',
  MSG_TX_SET_CAN = 'c',
  MSG_TX_SET_LOGGING = 'l',
  MSG_TX_SET_INPUTS = 'i',
  MSG_TX_SET_OUTPUTS = 'o',
  MSG_TX_SET_VIRTUAL_INPUTS = 'u',
  MSG_TX_SET_WIPER = 'w',
  MSG_TX_SET_WIPER_SPEED = 'p',
  MSG_TX_SET_WIPER_DELAYS = 'y',
  MSG_TX_SET_FLASHER = 'h',
  MSG_TX_SET_STARTER = 'd',
  MSG_TX_SET_CAN_INPUTS = 'n',
  MSG_TX_SET_TIME = 't',
  MSG_TX_GET_VERSION = 'v'

} MsgQueueTxCmd_t;

typedef struct
{
  MsgQueueRxSrc_t eMsgSrc;
  CAN_RxHeaderTypeDef stCanRxHeader;
  uint8_t nRxData[8];
  uint8_t nRxLen;
  uint32_t nCRC;
} MsgQueueRx_t;

typedef struct{
  uint8_t nTxData[8];
  uint8_t nTxLen;
} MsgQueueUsbTx_t;

typedef struct{
  CAN_TxHeaderTypeDef stTxHeader;
  uint8_t nTxData[8];
} MsgQueueCanTx_t;

#endif /* INC_MSG_QUEUE_H_ */
