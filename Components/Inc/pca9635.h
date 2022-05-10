/*
 * PCA9635.h
 *
 *  Created on: Jul 30, 2020
 *      Author: coryg
 */

#ifndef SRC_PCA9635_DRIVER_PCA9635_H_
#define SRC_PCA9635_DRIVER_PCA9635_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

#define PCA9635_REG_MODE1       0x00
#define PCA9635_REG_MODE2       0x01
#define PCA9635_REG_PWM(x)      (0x02+(x))

#define PCA9635_REG_GRPPWM      0x12
#define PCA9635_REG_GRPFREQ     0x13

#define PCA9635_REG_AI_ALL      0x80

// check datasheet for details
#define PCA9635_REG_LEDOUT_BASE 0x14    // 0x14..0x17
#define PCA9635_LEDOFF      0x00    // default @ startup
#define PCA9635_LEDON       0x01
#define PCA9635_LEDPWM      0x02
#define PCA9635_LEDGRPPWM   0x03

#define PCA9635_MODE1_ALLCALL  0x01
#define PCA9635_MODE1_SUB3     0x02
#define PCA9635_MODE1_SUB2     0x04
#define PCA9635_MODE1_SUB1     0x08
#define PCA9635_MODE1_SLEEP    0x10
#define PCA9635_MODE1_AI0      0X20
#define PCA9635_MODE1_AI1      0X40
#define PCA9635_MODE1_AI2      0x80

#define PCA9635_MODE2_OUTNE    0x01
#define PCA9635_MODE2_OUTDRV   0x04
#define PCA9635_MODE2_OCH      0x08
#define PCA9635_MODE2_INVERT   0x10
#define PCA9635_MODE2_DMBLNK   0x20

#define PCA9635_PWM 0
#define PCA9635_BLINK 1

typedef enum{
  LED_OFF = PCA9635_LEDOFF,
  LED_ON = PCA9635_LEDON,
  LED_PWM = PCA9635_LEDPWM,
  LED_FLASH = PCA9635_LEDGRPPWM
} PCA9635_LEDOnState_t;

void PCA9635_Init(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t blinking);
void PCA9635_SetPWM(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t channel, uint8_t value);
void PCA9635_SetGroupPWM(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t value);
void PCA9635_SetGroupFreq(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t value);
void PCA9635_SetAllNum(I2C_HandleTypeDef* hi2c, uint16_t addr, uint32_t values);
void PCA9635_SetAll(I2C_HandleTypeDef* hi2c, uint16_t addr, PCA9635_LEDOnState_t state[16]);

#endif /* SRC_PCA9635_DRIVER_PCA9635_H_ */
