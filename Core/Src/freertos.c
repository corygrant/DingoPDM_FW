/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dingo_pdm.h"
#include "msg_queue.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MSGQUEUE_RX_SIZE 16
#define MSGQUEUE_TX_SIZE 16
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc4;
CAN_HandleTypeDef hcan;
CRC_HandleTypeDef hcrc;
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;
IWDG_HandleTypeDef hiwdg;
RTC_HandleTypeDef hrtc;
SPI_HandleTypeDef hspi1;

#if( configGENERATE_RUN_TIME_STATS == 1)
  PRIVILEGED_DATA volatile static uint32_t nRunTimeCount = 0UL;
#endif

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for i2cTask */
osThreadId_t i2cTaskHandle;
const osThreadAttr_t i2cTask_attributes = {
  .name = "i2cTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for canTxTask */
osThreadId_t canTxTaskHandle;
const osThreadAttr_t canTxTask_attributes = {
  .name = "canTxTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for KickIWDG */
osTimerId_t KickIWDGHandle;
const osTimerAttr_t KickIWDG_attributes = {
  .name = "KickIWDG"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void IncrementRuntimeStats(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartI2CTask(void *argument);
void StartCanTxTask(void *argument);
//void KickIWDGCallback(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of KickIWDG */
  //KickIWDGHandle = osTimerNew(KickIWDGCallback, osTimerPeriodic, NULL, &KickIWDG_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  //IWDG resets at 1.6s
  // (Prescaler * (Reload+1)) / (LSI Freq)
  // (16 * 4096) / 40000 = 1.6384s
  // Window is set at 1s
  // Must kick between 1.0s to 1.6s
  //if(osTimerStart(KickIWDGHandle, 1300) != osOK)
  //  Error_Handler();

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  qMsgQueueRx = osMessageQueueNew(MSGQUEUE_RX_SIZE, sizeof(MsgQueueRx_t), NULL);
  if(qMsgQueueRx == NULL){
    //TODO: Message queue not created
    Error_Handler();
  }

  qMsgQueueUsbTx = osMessageQueueNew(MSGQUEUE_TX_SIZE, sizeof(MsgQueueUsbTx_t), NULL);
  if(qMsgQueueUsbTx == NULL){
    //TODO: Message queue not created
    Error_Handler();
  }

  qMsgQueueCanTx = osMessageQueueNew(MSGQUEUE_TX_SIZE, sizeof(MsgQueueCanTx_t), NULL);
  if(qMsgQueueCanTx == NULL){
    //TODO: Message queue not created
    Error_Handler();
  }
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of i2cTask */
  i2cTaskHandle = osThreadNew(StartI2CTask, NULL, &i2cTask_attributes);

  /* creation of canTxTask */
  canTxTaskHandle = osThreadNew(StartCanTxTask, NULL, &canTxTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  if(defaultTaskHandle == 0x0)
    Error_Handler();

  if(i2cTaskHandle == 0x0)
    Error_Handler();

  if(canTxTaskHandle == 0x0)
    Error_Handler();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  if(ReadPdmConfig() != PDM_OK)
    Error_Handler();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  if(ReadPdmConfig() != PDM_OK)
    Error_Handler();
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  PdmMainTask(&defaultTaskHandle, &hadc1, &hadc4, &hrtc, &hcrc);
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartI2CTask */
/**
* @brief Function implementing the i2cTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartI2CTask */
void StartI2CTask(void *argument)
{
  /* USER CODE BEGIN StartI2CTask */
  I2CTask(&i2cTaskHandle, &hi2c1, &hi2c2);
  /* USER CODE END StartI2CTask */
}

/* USER CODE BEGIN Header_StartCanTxTask */
/**
* @brief Function implementing the canTxTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartCanTxTask */
void StartCanTxTask(void *argument)
{
  /* USER CODE BEGIN StartCanTxTask */
  CanTxTask(&canTxTaskHandle, &hcan);
  /* USER CODE END StartCanTxTask */
}

/* KickIWDGCallback function */
void KickIWDGCallback(void *argument)
{
  /* USER CODE BEGIN KickIWDGCallback */
  HAL_IWDG_Refresh(&hiwdg);
  /* USER CODE END KickIWDGCallback */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
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
/* USER CODE END Application */

