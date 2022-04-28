/*
 * pcal9554b.h
 *
 *  Created on: Apr 27, 2022
 *      Author: coryg
 */

#ifndef COMPONENTS_INC_PCAL9554B_H_
#define COMPONENTS_INC_PCAL9554B_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

#define PCAL9554B_CMD_IN_PORT               0X00
#define PCAL9554B_CMD_OUT_PORT              0X01
#define PCAL9554B_CMD_POL_INV               0X02
#define PCAL9554B_CMD_CFG                   0X03
#define PCAL9554B_CMD_OUT_DRIVE_STRENGTH0   0X40
#define PCAL9554B_CMD_OUT_DRIVE_STRENGTH1   0X41
#define PCAL9554B_CMD_IN_LATCH              0X42
#define PCAL9554B_CMD_PU_PD_ENABLE          0X43
#define PCAL9554B_CMD_PU_PD_SELECT          0X44
#define PCAL9554B_CMD_INT_MASK              0X45
#define PCAL9554B_CMD_INT_STATUS            0X46
#define PCAL9554B_CMD_OUT_PORT_CFG          0X4F

void PCAL9554B_WriteReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint8_t val);
uint8_t PCAL9554B_ReadReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg);

#endif /* COMPONENTS_INC_PCAL9554B_H_ */
