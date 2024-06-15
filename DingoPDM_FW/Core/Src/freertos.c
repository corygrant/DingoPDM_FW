#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "dingo_pdm.h"
#include "msg_queue.h"

#define MSGQUEUE_RX_SIZE 64
#define MSGQUEUE_TX_SIZE 64

ADC_HandleTypeDef hadc1;
CAN_HandleTypeDef hcan1;
I2C_HandleTypeDef hi2c1;
CRC_HandleTypeDef hcrc;

#if( configGENERATE_RUN_TIME_STATS == 1)
  static PRIVILEGED_DATA volatile uint32_t nRunTimeCount = 0UL;
#endif

osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t canTxTaskHandle;
const osThreadAttr_t canTxTask_attributes = {
  .name = "canTxTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};

osTimerId_t KickIWDGHandle;
const osTimerAttr_t KickIWDG_attributes = {
  .name = "KickIWDG"
};

void IncrementRuntimeStats(void);

void StartDefaultTask(void *argument);
void StartCanTxTask(void *argument);
//void KickIWDGCallback(void *argument);

void MX_FREERTOS_Init(void);

void MX_FREERTOS_Init(void) {
 
  //KickIWDGHandle = osTimerNew(KickIWDGCallback, osTimerPeriodic, NULL, &KickIWDG_attributes);

  qMsgQueueRx = osMessageQueueNew(MSGQUEUE_RX_SIZE, sizeof(MsgQueueRx_t), NULL);
  if(qMsgQueueRx == NULL){
    Error_Handler(FATAL_ERROR_MSG_QUEUE);
  }

  qMsgQueueTx = osMessageQueueNew(MSGQUEUE_TX_SIZE, sizeof(MsgQueueTx_t), NULL);
  if(qMsgQueueTx == NULL){
	  Error_Handler(FATAL_ERROR_MSG_QUEUE);
  }


  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  canTxTaskHandle = osThreadNew(StartCanTxTask, NULL, &canTxTask_attributes);

  if(defaultTaskHandle == 0x0)
    Error_Handler(FATAL_ERROR_TASK);

  if(canTxTaskHandle == 0x0)
    Error_Handler(FATAL_ERROR_TASK);

  if(InitPdmConfig(&hi2c1) != PDM_OK)
    Error_Handler(FATAL_ERROR_CONFIG);

}

void StartDefaultTask(void *argument)
{
  PdmMainTask(&defaultTaskHandle, &hadc1, &hi2c1);
}

void StartCanTxTask(void *argument)
{
  CanTxTask(&canTxTaskHandle, &hcan1);
}

void KickIWDGCallback(void *argument)
{
  //HAL_IWDG_Refresh(&hiwdg);
}

#if( configGENERATE_RUN_TIME_STATS == 1)
void ConfigureRunTimeCounter(void);
uint32_t GetRunTimeCounter(void);

void ConfigureRunTimeCounter(void)
{
  nRunTimeCount = 0;
}

uint32_t GetRunTimeCounter(void)
{
  return nRunTimeCount;
}

void IncrementRuntimeStats(void)
{
  nRunTimeCount = nRunTimeCount + 1;
}
#endif

