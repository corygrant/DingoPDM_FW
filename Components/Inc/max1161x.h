/*
 * max1161x.h
 *
 *  Created on: Mar 21, 2023
 *      Author: coryg
 */

#ifndef INC_MAX1161X_H_
#define INC_MAX1161X_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define MAX11613_ADDRESS (0x34) ///< 0110 100 (ADDR = GND)
/*=========================================================================*/

/*=========================================================================
    CONVERSION DELAY (in mS)
    -----------------------------------------------------------------------*/
#define MAX11613_CONVERSIONDELAY (1) ///< Conversion delay
/*=========================================================================*/

/*=========================================================================
    SETUP REGISTERS
    -----------------------------------------------------------------------*/
//BIT 7
#define MAX11613_REG_SETUP          (0x80)

//BITS 4-6
#define MAX11613_SEL_MASK           (0x70)
#define MAX11613_SEL0_INT_REF_ON    (0x10)
#define MAX11613_SEL0_INT_REF_OFF   (0x00)
#define MAX11613_SEL1_AIN_REFERENCE (0x20)
#define MAX11613_SEL1_AIN_INPUT     (0x00)
#define MAX11613_SEL2_REF_INTERNAL  (0x40)
#define MAX11613_SEL2_REF_EXTERNAL  (0x00)

//BIT 3
#define MAX11613_CLK_EXTERNAL (0x08)
#define MAX11613_CLK_INTERNAL (0x00)

//BIT 2
#define MAX11613_BIPOLAR      (0x04)
#define MAX11613_UNIPOLAR     (0x00)

//BIT 1
#define MAX11613_RST_NO_ACTION  (0x02)
#define MAX11613_RST_DEFAULT    (0x00)
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTERS
    -----------------------------------------------------------------------*/
//BIT 7
#define MAX11613_REG_CONFIG         (0x00)

//BITS 5-6
#define MAX11613_SCAN_MASK          (0x60)
#define MAX11613_SCAN_8X            (0x20)
#define MAX11613_SCAN_UPPER         (0x40)
#define MAX11613_SCAN_SELECTED      (0x60)

//BITS 1-4
#define MAX11613_CH_SELECT_MASK     (0x06)
#define MAX11613_CH0                (0x00)
#define MAX11613_CH1                (0x02)
#define MAX11613_CH2                (0x04)
#define MAX11613_CH3                (0x06)

//BIT 0
#define MAX11613_SINGLE_ENDED         (0x01)
#define MAX11613_DIFFERENTIAL         (0x00)

HAL_StatusTypeDef MAX1161x_SendSetup(I2C_HandleTypeDef* hi2c, uint16_t addr);
HAL_StatusTypeDef MAX1161x_ReadADC(I2C_HandleTypeDef* hi2c, uint16_t addr, uint8_t channel, uint16_t* val);
#endif /* INC_MAX1161X_H_ */
