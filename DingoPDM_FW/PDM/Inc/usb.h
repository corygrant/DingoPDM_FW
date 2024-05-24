#ifndef INC_USB_H_
#define INC_USB_H_

#include "stdint.h"
#include <stdbool.h>
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

#define USBD_RX_DATA_SIZE  2048
#define USBD_TX_DATA_SIZE  2048

typedef void (*rcvCallback_t)(uint8_t* Buf, uint32_t *Len);

uint8_t USB_Init(void (*rcvFunction)(uint8_t* Buf, uint32_t *Len));
uint8_t USB_Tx(uint8_t* Buf, uint16_t Len);
uint8_t USB_Tx_SLCAN(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[]);
bool USB_IsConnected();
#endif /* INC_USB_H_ */