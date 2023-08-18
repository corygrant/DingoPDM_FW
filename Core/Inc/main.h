/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define CANx_RX_IRQn                   CAN1_RX0_IRQn
#define CANx_RX_IRQHandler             CAN1_RX0_IRQHandler
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

//**************
//See stm32f3xx.h for other bit macros
//**************
#define BIT_MASK_AT(pos)         (1 << (pos))
#define SET_BIT_AT(value,pos)    ((value) |= BIT_MASK_AT(pos))
#define CLEAR_BIT_AT(value,pos)  ((value) &= ~BIT_MASK_AT(pos))
#define GET_BIT_AT(value,pos)    (((value) & BIT_MASK_AT(pos)) ? 1 : 0)
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PF_DEN2_Pin GPIO_PIN_13
#define PF_DEN2_GPIO_Port GPIOC
#define PF_IN2_Pin GPIO_PIN_14
#define PF_IN2_GPIO_Port GPIOC
#define EXTRA1_Pin GPIO_PIN_15
#define EXTRA1_GPIO_Port GPIOC
#define PF_IS2_Pin GPIO_PIN_0
#define PF_IS2_GPIO_Port GPIOA
#define PF_IS3_4_Pin GPIO_PIN_1
#define PF_IS3_4_GPIO_Port GPIOA
#define PF_IS1_Pin GPIO_PIN_2
#define PF_IS1_GPIO_Port GPIOA
#define PF_IS5_6_Pin GPIO_PIN_3
#define PF_IS5_6_GPIO_Port GPIOA
#define PF_IS7_8_Pin GPIO_PIN_4
#define PF_IS7_8_GPIO_Port GPIOA
#define PF_IN8_Pin GPIO_PIN_5
#define PF_IN8_GPIO_Port GPIOA
#define PF_DSEL7_8_Pin GPIO_PIN_6
#define PF_DSEL7_8_GPIO_Port GPIOA
#define PF_DSEL7_8A7_Pin GPIO_PIN_7
#define PF_DSEL7_8A7_GPIO_Port GPIOA
#define PF_IN7_Pin GPIO_PIN_0
#define PF_IN7_GPIO_Port GPIOB
#define PF_IN6_Pin GPIO_PIN_1
#define PF_IN6_GPIO_Port GPIOB
#define PF_DSEL5_6_Pin GPIO_PIN_2
#define PF_DSEL5_6_GPIO_Port GPIOB
#define PF_DSEL5_6B10_Pin GPIO_PIN_10
#define PF_DSEL5_6B10_GPIO_Port GPIOB
#define PF_IN5_Pin GPIO_PIN_11
#define PF_IN5_GPIO_Port GPIOB
#define BATT_SENSE_Pin GPIO_PIN_12
#define BATT_SENSE_GPIO_Port GPIOB
#define DIG_IN2_Pin GPIO_PIN_13
#define DIG_IN2_GPIO_Port GPIOB
#define DIG_IN1_Pin GPIO_PIN_14
#define DIG_IN1_GPIO_Port GPIOB
#define PF_DEN1_Pin GPIO_PIN_15
#define PF_DEN1_GPIO_Port GPIOB
#define PF_IN1_Pin GPIO_PIN_9
#define PF_IN1_GPIO_Port GPIOA
#define PF_IN4_Pin GPIO_PIN_10
#define PF_IN4_GPIO_Port GPIOA
#define PF_DSEL3_4_Pin GPIO_PIN_15
#define PF_DSEL3_4_GPIO_Port GPIOA
#define PF_DEN3_4_Pin GPIO_PIN_4
#define PF_DEN3_4_GPIO_Port GPIOB
#define PF_IN3_Pin GPIO_PIN_5
#define PF_IN3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
