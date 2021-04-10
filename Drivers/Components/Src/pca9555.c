/*
 * pca9555.c
 *
 *  Created on: Nov 1, 2020
 *      Author: coryg
 */

#include "pca9555.h"

void PCA9555_WriteReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint8_t val)
{
  uint8_t writeVals[2];

  writeVals[0] = reg;
  writeVals[1] = val;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, HAL_MAX_DELAY);
}

uint8_t PCA9555_ReadReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg)
{
  uint8_t writeVals[1];
  uint8_t readVals[1];

  uint8_t val = 0xFF;

  writeVals[0] = reg;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 1, HAL_MAX_DELAY);

  HAL_I2C_Master_Receive(hi2c, addr << 1, readVals, 1, HAL_MAX_DELAY);

  val = readVals[0];

  return val;
}

void PCA9555_WriteReg16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint16_t val)
{
  uint8_t writeVals[3];

  writeVals[0] = reg;
  writeVals[1] = val & 0xFF;
  writeVals[2] = val >> 8;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 3, HAL_MAX_DELAY);
}

uint8_t PCA9555_ReadReg16(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg)
{
  uint8_t writeVals[1];
  uint8_t readVals[2];
  uint16_t val = 0xFFFF;

  writeVals[0] = reg;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 1, HAL_MAX_DELAY);

  HAL_I2C_Master_Receive(hi2c, addr << 1, readVals, 2, HAL_MAX_DELAY);

  val = (readVals[0] << 8 | readVals[1]);

  return val;
}
