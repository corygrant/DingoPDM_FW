#include "can.h"

void CAN_Init(CAN_HandleTypeDef* canHandle, CanSpeed_t eSpeed)
{
  HAL_CAN_MspInit(canHandle);

  //Configure the CAN peripheral with the correct speed
  switch(eSpeed){
    case BITRATE_1000K:
      canHandle->Init.Prescaler = 2;
      break;
    case BITRATE_500K:
      canHandle->Init.Prescaler = 4;
      break;
    case BITRATE_250K:
      canHandle->Init.Prescaler = 8;
      break;
    default: //500K
      canHandle->Init.Prescaler = 4;
      break;
  }

  canHandle->Instance = CAN1;
  canHandle->Init.Mode = CAN_MODE_NORMAL;
  canHandle->Init.SyncJumpWidth = CAN_SJW_1TQ;
  canHandle->Init.TimeSeg1 = CAN_BS1_15TQ;
  canHandle->Init.TimeSeg2 = CAN_BS2_2TQ;
  canHandle->Init.TimeTriggeredMode = DISABLE;
  canHandle->Init.AutoBusOff = DISABLE;
  canHandle->Init.AutoWakeUp = DISABLE;
  canHandle->Init.AutoRetransmission = DISABLE;
  canHandle->Init.ReceiveFifoLocked = DISABLE;
  canHandle->Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(canHandle) != HAL_OK)
  {
    Error_Handler(FATAL_ERROR_CAN);
  }
}


void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
 
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
 
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{
  if(canHandle->Instance==CAN1)
  {
    __HAL_RCC_CAN1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
  }
}

