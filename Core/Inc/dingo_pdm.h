/*
 * dingo_pdm.h
 *
 *  Created on: Oct 22, 2020
 *      Author: coryg
 */

#ifndef INC_DINGO_PDM_H_
#define INC_DINGO_PDM_H_

#include <stdbool.h>
#include "stdio.h"
#include "cmsis_os.h"

#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

#include "msg_queue.h"
#include "profet.h"
#include "ads1x15.h"
#include "mcp9808.h"
#include "pca9539.h"
#include "pcal9554b.h"
#include "pca9635.h"
#include "mb85rc.h"
#include "logger.h"
#include "profet.h"
#include "canboard.h"
#include "wipers.h"
#include "pushbutton.h"
#include "can_input.h"
#include "pdm_input.h"
#include "virtual_input.h"
#include "starter.h"
#include "flasher.h"

//#define MEAS_HEAP_USE

#define USBD_RX_DATA_SIZE  2048
#define USBD_TX_DATA_SIZE  2048

#define PDM_NOK 0
#define PDM_OK 1

typedef enum
{
  DEVICE_AUTO,
  DEVICE_MANUAL
} DeviceMode_t;


typedef enum
{
  DEVICE_INIT,
  DEVICE_RUN,
  DEVICE_CONFIG,
  DEVICE_ERROR
} DeviceState_t;


typedef enum{
  FORCE_STEADY,
  FORCE_INTER,
} IOForceMode_t;

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

void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, ADC_HandleTypeDef* hadc4, CAN_HandleTypeDef* hcan, RTC_HandleTypeDef* hrtc, CRC_HandleTypeDef* hcrc);
void I2CTask(osThreadId_t* thisThreadId, I2C_HandleTypeDef* hi2c1, I2C_HandleTypeDef* hi2c2);
void ProfetSMTask(osThreadId_t* thisThreadId, I2C_HandleTypeDef* hi2c2, RTC_HandleTypeDef* hrtc);
void CanTxTask(osThreadId_t* thisThreadId, CAN_HandleTypeDef* hcan);
void Profet_Init();
void SetPfStatusLed(PCA9635_LEDOnState_t *ledState, volatile ProfetTypeDef *profet);
void GetRTCTimeDate(RTC_HandleTypeDef* hrtc, uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second);

uint8_t ReadPdmConfig(I2C_HandleTypeDef* hi2c2);
#endif /* INC_DINGO_PDM_H_ */
