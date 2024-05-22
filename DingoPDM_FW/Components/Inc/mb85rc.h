#ifndef COMPONENTS_INC_MB85RC_H_
#define COMPONENTS_INC_MB85RC_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"

#define MB85RC_SLAVE_ID 0xF8
#define MB85RC_MANUF_ID 0x00A
#define MB85RC_PROD_ID 0x510

HAL_StatusTypeDef MB85RC_GetId(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t* nManufId, uint16_t* nProdId);
HAL_StatusTypeDef MB85RC_CheckId(I2C_HandleTypeDef* hi2c, uint8_t nAddr);
HAL_StatusTypeDef MB85RC_Read(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t nMemAddr, uint8_t* pData, uint16_t nByteLen);
HAL_StatusTypeDef MB85RC_Write(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t nMemAddr, uint8_t* nMemVals, uint16_t nByteLen);
#endif /* COMPONENTS_INC_MB85RC_H_ */
