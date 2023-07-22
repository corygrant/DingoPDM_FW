/*
 * max1161x.c
 *
 *  Created on: Mar 21, 2023
 *      Author: coryg
 */

#include "max1161x.h"

#define I2C_TIMEOUT 100

HAL_StatusTypeDef MAX1161x_SendSetup(I2C_HandleTypeDef* hi2c, uint16_t addr)
{
  HAL_StatusTypeDef eStatus;

  uint8_t setupVal =
      MAX11613_REG_SETUP |
      MAX11613_SEL1_AIN_INPUT |
      MAX11613_SEL2_REF_INTERNAL |
      MAX11613_SEL0_INT_REF_ON |
      MAX11613_CLK_EXTERNAL |
      MAX11613_RST_NO_ACTION;

  eStatus = HAL_I2C_Master_Transmit(hi2c, addr << 1, &setupVal, 1, I2C_TIMEOUT);

  return eStatus;
}

HAL_StatusTypeDef MAX1161x_ReadADC(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t channel, uint16_t* val)
{
  HAL_StatusTypeDef eStatus;

  if(channel > 3) return HAL_ERROR;

  uint8_t configVal =
      MAX11613_REG_CONFIG |
      MAX11613_SCAN_8X |
      (MAX11613_CH_SELECT_MASK & (channel << 1)) |
      MAX11613_SINGLE_ENDED;

  eStatus = HAL_I2C_Master_Transmit(hi2c, addr << 1, &configVal, 1, I2C_TIMEOUT);

  if( eStatus != HAL_OK)
  {
    return eStatus;
  }

  //Read received values
  uint8_t readVals[2];

  eStatus = HAL_I2C_Master_Receive(hi2c, addr << 1, readVals, 2, I2C_TIMEOUT);
  if( eStatus != HAL_OK)
  {
    return eStatus;
  }

  //Device reads channel 8 times, average it
  uint16_t valRead = (readVals[0] << 8 | readVals[1]);
      //(readVals[2] << 8 | readVals[3]) +
      //(readVals[4] << 8 | readVals[5]) +
      //(readVals[6] << 8 | readVals[7]) +
      //(readVals[8] << 8 | readVals[9]) +
      //(readVals[10] << 8 | readVals[11]) +
      //(readVals[12] << 8 | readVals[13]) +
      //(readVals[14] << 8 | readVals[15]);

  valRead = valRead & 0xFFF; //12 bit

  *val = valRead; //Right shift 3 = divide by 8

  return HAL_OK;
}

