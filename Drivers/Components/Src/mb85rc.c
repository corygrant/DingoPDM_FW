/*
 * mb85rc.c
 *
 *  Created on: Jan 11, 2021
 *      Author: coryg
 */

#include "mb85rc.h"

void MB85RC_GetId(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t* nManufId, uint16_t* nProdId)
{
  uint8_t nReadVals[3];

  HAL_I2C_Mem_Read(hi2c, MB85RC_SLAVE_ID, nAddr << 1, I2C_MEMADD_SIZE_8BIT, &nReadVals[0], 3, 100);

  *nManufId = (nReadVals[0] << 4) + (nReadVals[1] >> 4);
  *nProdId = ((nReadVals[1] & 0x0F) << 8) + nReadVals[2];
}

uint8_t MB85RC_CheckId(I2C_HandleTypeDef* hi2c, uint8_t nAddr)
{
  uint16_t nManufId, nProdId;

  MB85RC_GetId(hi2c, nAddr, &nManufId, &nProdId);

  if(nManufId != MB85RC_MANUF_ID)
    return 0;
  if(nProdId != MB85RC_PROD_ID)
    return 0;

  return 1;
}

void MB85RC_Read(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t nMemAddr, uint8_t* pData, uint16_t nByteLen)
{
  HAL_I2C_Mem_Read(hi2c, nAddr << 1, (uint16_t)nMemAddr, I2C_MEMADD_SIZE_16BIT, pData, nByteLen, HAL_MAX_DELAY);
}

void MB85RC_Write(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t nMemAddr, uint8_t* nMemVals, uint16_t nByteLen)
{
  HAL_I2C_Mem_Write(hi2c, nAddr << 1, (uint16_t)nMemAddr, I2C_MEMADD_SIZE_16BIT, nMemVals, nByteLen, HAL_MAX_DELAY);
}
