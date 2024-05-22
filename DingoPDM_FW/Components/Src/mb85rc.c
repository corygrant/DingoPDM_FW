#include "mb85rc.h"

#define I2C_TIMEOUT 100

HAL_StatusTypeDef MB85RC_GetId(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t* nManufId, uint16_t* nProdId)
{
  HAL_StatusTypeDef eStatus;
  uint8_t nReadVals[3];

  eStatus = HAL_I2C_Mem_Read(hi2c, MB85RC_SLAVE_ID, nAddr << 1, I2C_MEMADD_SIZE_8BIT, &nReadVals[0], 3, I2C_TIMEOUT);
  if( eStatus != HAL_OK)
  {
    return eStatus;
  }

  *nManufId = (nReadVals[0] << 4) + (nReadVals[1] >> 4);
  *nProdId = ((nReadVals[1] & 0x0F) << 8) + nReadVals[2];
  return HAL_OK;
}

HAL_StatusTypeDef MB85RC_CheckId(I2C_HandleTypeDef* hi2c, uint8_t nAddr)
{
  HAL_StatusTypeDef eStatus;
  uint16_t nManufId, nProdId;

  eStatus = MB85RC_GetId(hi2c, nAddr, &nManufId, &nProdId);
  if( eStatus != HAL_OK)
  {
    return eStatus;
  }

  if(nManufId != MB85RC_MANUF_ID)
    return HAL_ERROR;
  if(nProdId != MB85RC_PROD_ID)
    return HAL_ERROR;

  return HAL_OK;
}

HAL_StatusTypeDef MB85RC_Read(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t nMemAddr, uint8_t* pData, uint16_t nByteLen)
{
  return HAL_I2C_Mem_Read(hi2c, nAddr << 1, (uint16_t)nMemAddr, I2C_MEMADD_SIZE_16BIT, pData, nByteLen, I2C_TIMEOUT);
}

HAL_StatusTypeDef MB85RC_Write(I2C_HandleTypeDef* hi2c, uint8_t nAddr, uint16_t nMemAddr, uint8_t* nMemVals, uint16_t nByteLen)
{
  return HAL_I2C_Mem_Write(hi2c, nAddr << 1, (uint16_t)nMemAddr, I2C_MEMADD_SIZE_16BIT, nMemVals, nByteLen, I2C_TIMEOUT);
}
