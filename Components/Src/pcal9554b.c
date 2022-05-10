/*
 * pcal9554b.c
 *
 *  Created on: Apr 27, 2022
 *      Author: coryg
 */

#include "pcal9554b.h"

void PCAL9554B_WriteReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg, uint8_t val)
{
  uint8_t writeVals[2];

  writeVals[0] = reg;
  writeVals[1] = val;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, 100);
}

uint8_t PCAL9554B_ReadReg8(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t reg)
{
  uint8_t writeVals[1];
  uint8_t readVals[1];

  uint8_t val = 0xFF;

  writeVals[0] = reg;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 1, 100);

  HAL_I2C_Master_Receive(hi2c, addr << 1, readVals, 1, 100);

  val = readVals[0];

  return val;
}
