#ifndef INC_USB_H_
#define INC_USB_H_

#include "stdint.h"
#include <stdbool.h>
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

#include "cmsis_os.h"
#include "msg_queue.h"

#define SEND_ALL_USB

#define USBD_RX_DATA_SIZE  2048
#define USBD_TX_DATA_SIZE  2048

extern osMessageQueueId_t qMsgQueueRx;
extern USBD_CDC_ItfTypeDef USBD_Interface_PDM;

uint8_t USB_InitStart(uint16_t nBaseId);
void USB_UpdateID(uint16_t nBaseId);
static int8_t USBD_CDC_Init(void);
static int8_t USBD_CDC_DeInit(void);
static int8_t USBD_CDC_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t USBD_CDC_Receive(uint8_t* Buf, uint32_t *Len);
uint8_t USBD_CDC_Transmit(uint8_t* Buf, uint16_t Len);
uint8_t USBD_CDC_Transmit_SLCAN(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[]);

#endif /* INC_USB_H_ */