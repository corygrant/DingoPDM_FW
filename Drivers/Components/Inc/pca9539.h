/*
 * pca9539.h
 *
 *  Created on: Apr 27, 2022
 *      Author: coryg
 */

#ifndef COMPONENTS_INC_PCA9539_H_
#define COMPONENTS_INC_PCA9539_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

#define PCA9539_CMD_IN_PORT0 0X0
#define PCA9539_CMD_IN_PORT1 0X1
#define PCA9539_CMD_OUT_PORT0 0X2
#define PCA9539_CMD_OUT_PORT1 0X3
#define PCA9539_CMD_POLARITY_INVERT_PORT0 0X4
#define PCA9539_CMD_POLARITY_INVERT_PORT1 0X5
#define PCA9539_CMD_CONFIG_PORT0 0X6
#define PCA9539_CMD_CONFIG_PORT1 0X7

void PCA9539_WriteReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint8_t val);
uint8_t PCA9539_ReadReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg);
void PCA9539_WriteReg16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint16_t val);
uint8_t PCA9539_ReadReg16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg);

#endif /* COMPONENTS_INC_PCA9539_H_ */
