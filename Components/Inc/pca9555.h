/*
 * pca9555.h
 *
 *  Created on: Oct 30, 2020
 *      Author: coryg
 */

#ifndef INC_PCA9555_H_
#define INC_PCA9555_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

#define PCA9555_CMD_IN_PORT0 0X0
#define PCA9555_CMD_IN_PORT1 0X1
#define PCA9555_CMD_OUT_PORT0 0X2
#define PCA9555_CMD_OUT_PORT1 0X3
#define PCA9555_CMD_POLARITY_INVERT_PORT0 0X4
#define PCA9555_CMD_POLARITY_INVERT_PORT1 0X5
#define PCA9555_CMD_CONFIG_PORT0 0X6
#define PCA9555_CMD_CONFIG_PORT1 0X7

void PCA9555_WriteReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint8_t val);
uint8_t PCA9555_ReadReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg);
void PCA9555_WriteReg16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint16_t val);
uint8_t PCA9555_ReadReg16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg);

#endif /* INC_PCA9555_H_ */
