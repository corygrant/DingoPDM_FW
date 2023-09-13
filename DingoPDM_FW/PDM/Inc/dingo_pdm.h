/*
 * dingo_pdm.h
 *
 *  Created on: Oct 22, 2020
 *      Author: coryg
 */

#ifndef INC_DINGO_PDM_H_
#define INC_DINGO_PDM_H_

#include <can_input.h>
#include <input.h>
#include <stdbool.h>
#include "stdio.h"
#include "cmsis_os.h"

#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

#include "msg_queue.h"
#include "pdm_input.h"
#include "starter.h"
#include "flasher.h"
#include "canboard.h"
#include "mb85rc.h"
#include "mcp9808.h"
#include "profet.h"
#include "virtual_input.h"
#include "wipers.h"

//#define MEAS_HEAP_USE

#define SEND_ALL_USB

#define USBD_RX_DATA_SIZE  2048
#define USBD_TX_DATA_SIZE  2048

#define PDM_NOK 0
#define PDM_OK 1

typedef enum{
    CAN_BITRATE_10K = 0,
    CAN_BITRATE_20K = 1,
    CAN_BITRATE_50K = 2,
    CAN_BITRATE_100K = 3,
    CAN_BITRATE_125K = 4,
    CAN_BITRATE_250K = 5,
    CAN_BITRATE_500K = 6,
    CAN_BITRATE_750K = 7,
    CAN_BITRATE_1000K = 8,
    CAN_BITRATE_INVALID
} CAN_BitRate_t;

extern osMessageQueueId_t qMsgQueueRx;
extern osMessageQueueId_t qMsgQueueUsbTx;
extern osMessageQueueId_t qMsgQueueCanTx;

extern USBD_CDC_ItfTypeDef USBD_Interface_PDM;
uint8_t USBD_CDC_Transmit(uint8_t* Buf, uint16_t Len);
uint8_t USBD_CDC_Transmit_SLCAN(CAN_TxHeaderTypeDef *pHeader, uint8_t aData[]);

void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, I2C_HandleTypeDef* hi2c1);
void CanTxTask(osThreadId_t* thisThreadId, CAN_HandleTypeDef* hcan);
uint8_t InitPdmConfig(I2C_HandleTypeDef* hi2c1);
#endif /* INC_DINGO_PDM_H_ */
