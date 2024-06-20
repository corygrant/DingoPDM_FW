#ifndef __CAN_H__
#define __CAN_H__

#include "main.h"

extern CAN_HandleTypeDef hcan1;


void CAN_Init(CAN_HandleTypeDef* canHandle, CanSpeed_t eSpeed);

#endif /* __CAN_H__ */

