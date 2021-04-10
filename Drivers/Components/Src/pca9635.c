/*
 * pca9635.C
 *
 *  Created on: Oct 30, 2020
 *      Author: coryg
 */


#include <pca9635.h>

void PCA9635_Init(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t blinking)
{
  uint8_t writeVals[2];

  writeVals[0] = PCA9635_REG_MODE1;
  writeVals[1] = (PCA9635_MODE1_ALLCALL | PCA9635_MODE1_AI2); //Auto increment all registers
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, HAL_MAX_DELAY);

  if(blinking > 0){
    writeVals[0] = PCA9635_REG_MODE2;
    writeVals[1] = (PCA9635_MODE2_OUTNE | PCA9635_MODE2_OUTDRV | PCA9635_MODE2_DMBLNK);
    HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, HAL_MAX_DELAY);
  }
}

void PCA9635_SetPWM(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t channel, uint8_t value)
{
  uint8_t writeVals[2];

  if((channel >= 0) && (channel < 16)){
    writeVals[0] = PCA9635_REG_PWM(channel);
    writeVals[1] = value;
    HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, HAL_MAX_DELAY);
  }
}

void PCA9635_SetGroupPWM(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t value)
{
  uint8_t writeVals[2];
  writeVals[0] = PCA9635_REG_GRPPWM;
  writeVals[1] = value;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, HAL_MAX_DELAY);
}

void PCA9635_SetGroupFreq(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t value)
{
  uint8_t writeVals[2];
  writeVals[0] = PCA9635_REG_GRPFREQ;
  writeVals[1] = value;
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 2, HAL_MAX_DELAY);
}

void PCA9635_SetAllNum(I2C_HandleTypeDef* hi2c, uint16_t addr, uint32_t values)
{
  uint8_t writeVals[5];
  writeVals[0] = (PCA9635_REG_LEDOUT_BASE | PCA9635_REG_AI_ALL);
  writeVals[1] = values & 0xFF;
  writeVals[2] = (values >> 8) ;
  writeVals[3] = (values >> 16);
  writeVals[4] = (values >> 24);
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 5, HAL_MAX_DELAY);
}

void PCA9635_SetAll(I2C_HandleTypeDef* hi2c, uint16_t addr, PCA9635_LEDOnState_t state[16])
{
  uint8_t writeVals[5];
  writeVals[0] = (PCA9635_REG_LEDOUT_BASE | PCA9635_REG_AI_ALL);
  writeVals[1] = state[0] + (state[1] << 2) + (state[2] << 4) + (state[3] << 6);
  writeVals[2] = state[4] + (state[5] << 2) + (state[6] << 4) + (state[7] << 6);
  writeVals[3] = state[8] + (state[9] << 2) + (state[10] << 4) + (state[11] << 6);
  writeVals[4] = state[12] + (state[13] << 2) + (state[14] << 4) + (state[15] << 6);
  HAL_I2C_Master_Transmit(hi2c, addr << 1, writeVals, 5, HAL_MAX_DELAY);
}
