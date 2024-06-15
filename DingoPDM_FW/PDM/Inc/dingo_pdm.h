#ifndef INC_DINGO_PDM_H_
#define INC_DINGO_PDM_H_


#include <stdbool.h>
#include "stdio.h"
#include "cmsis_os.h"
#include "main.h"

#include "pdm_config.h"
#include "can_input.h"
#include "input.h"
#include "msg_queue.h"
#include "pdm_input.h"
#include "starter.h"
#include "flasher.h"
#include "mb85rc.h"
#include "mcp9808.h"
#include "profet.h"
#include "virtual_input.h"
#include "wipers.h"
#include "status_led.h"
#include "usb.h"
#include "error.h"

//#define MEAS_HEAP_USE

#define PDM_NOK 0
#define PDM_OK 1

typedef enum{
  DEVICE_POWER_ON,
  DEVICE_STARTING,
  DEVICE_RUN,
  DEVICE_SLEEP,
  DEVICE_WAKEUP,
  DEVICE_OVERTEMP,
  DEVICE_ERROR
} DeviceState_t;

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

typedef enum{
  PDM_ERROR_FRAM_READ = 1,
  PDM_ERROR_FRAM_WRITE
} PdmErrorState_t;



extern osMessageQueueId_t qMsgQueueRx;
extern osMessageQueueId_t qMsgQueueTx;



void PdmMainTask(osThreadId_t* thisThreadId, ADC_HandleTypeDef* hadc1, I2C_HandleTypeDef* hi2c1);
void CanTxTask(osThreadId_t* thisThreadId, CAN_HandleTypeDef* hcan);
uint8_t InitPdmConfig(I2C_HandleTypeDef* hi2c1);
void ErrorState(uint8_t nErrorId);
#endif /* INC_DINGO_PDM_H_ */
