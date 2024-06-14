#ifndef INC_MSG_QUEUE_H_
#define INC_MSG_QUEUE_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"

typedef enum
{
  CAN_RX,
  USB_RX
} MsgQueueRxSrc_t;

typedef enum
{
  MSG_RX_BURN_SETTINGS = 'B',
  MSG_RX_SLEEP = 'Q',
  MSG_RX_SET_CAN = 'C',
  MSG_RX_SET_INPUTS = 'I',
  MSG_RX_SET_OUTPUTS = 'O',
  MSG_RX_SET_VIRTUAL_INPUTS = 'U',
  MSG_RX_SET_WIPER = 'W',
  MSG_RX_SET_WIPER_SPEED = 'P',
  MSG_RX_SET_WIPER_DELAYS = 'Y',
  MSG_RX_SET_FLASHER = 'H',
  MSG_RX_SET_STARTER = 'D',
  MSG_RX_SET_CAN_INPUTS = 'N',
  MSG_RX_GET_VERSION = 'V',
  MSG_RX_GET_TEMP = 'F'
} MsgQueueRxCmd_t;

typedef enum
{
  MSG_TX_BURN_SETTINGS = 'b',
  MSG_TX_SLEEP = 'q',
  MSG_TX_SET_CAN = 'c',
  MSG_TX_SET_INPUTS = 'i',
  MSG_TX_SET_OUTPUTS = 'o',
  MSG_TX_SET_VIRTUAL_INPUTS = 'u',
  MSG_TX_SET_WIPER = 'w',
  MSG_TX_SET_WIPER_SPEED = 'p',
  MSG_TX_SET_WIPER_DELAYS = 'y',
  MSG_TX_SET_FLASHER = 'h',
  MSG_TX_SET_STARTER = 'd',
  MSG_TX_SET_CAN_INPUTS = 'n',
  MSG_TX_GET_VERSION = 'v',
  MSG_TX_GET_TEMP = 'f'
} MsgQueueTxCmd_t;

typedef enum
{
  MSG_TYPE_INFO = 'F',
  MSG_TYPE_WARNING = 'R',
  MSG_TYPE_ERROR = 'E'
} MsgType_t;

typedef enum
{
  MSG_INFO_STATE_POWER_ON = 1,
  MSG_INFO_STATE_STARTING,
  MSG_INFO_STATE_RUN,
  MSG_INFO_STATE_SLEEP,
  MSG_INFO_STATE_WAKE,
  MSG_INFO_USB_CONNECTED
 } MsgInfo_t;

typedef enum
{
  MSG_WARNING_STATE_OVERTEMP = 1,
  MSG_WARNING_OVERCURRENT,
  MSG_WARNING_TEMP,
  MSG_WARNING_VOLTAGE,
  MSG_WARNING_CAN,
  MSG_WARNING_USB
 } MsgWarning_t;

typedef enum{
  MSG_ERROR_NONE = 0,
  MSG_ERROR_STATE_ERROR,
  MSG_ERROR_OVERCURRENT,
  MSG_ERROR_OVERTEMP,
  MSG_ERROR_IWDG,
  MSG_ERROR_MSG_QUEUE,
  MSG_ERROR_TASK,
  MSG_ERROR_CONFIG,
  MSG_ERROR_FRAM,
  MSG_ERROR_ADC,
  MSG_ERROR_TEMP_SENSOR,
  MSG_ERROR_USB,
  MSG_ERROR_CAN,
  MSG_ERROR_CRC,
  MSG_ERROR_I2C,
  MSG_ERROR_RCC
} MsgError_t;

typedef struct
{
  MsgQueueRxSrc_t eMsgSrc;
  CAN_RxHeaderTypeDef stCanRxHeader;
  uint8_t nRxData[8];
  uint8_t nRxLen;
  uint32_t nCRC;
} MsgQueueRx_t;

typedef struct{
  CAN_TxHeaderTypeDef stTxHeader;
  uint8_t nTxData[8];
} MsgQueueTx_t;

#endif /* INC_MSG_QUEUE_H_ */
