/*
 * ads1x15.h
 *
 *  Created on: Jul 28, 2020
 *      Author: coryg
 */

#ifndef SRC_ADS1X15_ADS1X15_H_
#define SRC_ADS1X15_ADS1X15_H_

#include "stdint.h"
#include "stm32f3xx_hal.h"

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define ADS1015_ADDRESS (0x48) ///< 1001 000 (ADDR = GND)
/*=========================================================================*/

/*=========================================================================
    CONVERSION DELAY (in mS)
    -----------------------------------------------------------------------*/
#define ADS1015_CONVERSIONDELAY (1) ///< Conversion delay
#define ADS1115_CONVERSIONDELAY (2) ///< Conversion delay
/*=========================================================================*/

/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
#define ADS1015_REG_POINTER_MASK (0x03)      ///< Point mask
#define ADS1015_REG_POINTER_CONVERT (0x00)   ///< Conversion
#define ADS1015_REG_POINTER_CONFIG (0x01)    ///< Configuration
#define ADS1015_REG_POINTER_LOWTHRESH (0x02) ///< Low threshold
#define ADS1015_REG_POINTER_HITHRESH (0x03)  ///< High threshold
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
#define ADS1015_REG_CONFIG_OS_MASK (0x8000) ///< OS Mask
#define ADS1015_REG_CONFIG_OS_SINGLE                                           \
  (0x8000) ///< Write: Set to start a single-conversion
#define ADS1015_REG_CONFIG_OS_BUSY                                             \
  (0x0000) ///< Read: Bit = 0 when conversion is in progress
#define ADS1015_REG_CONFIG_OS_NOTBUSY                                          \
  (0x8000) ///< Read: Bit = 1 when device is not performing a conversion

#define ADS1015_REG_CONFIG_MUX_MASK (0x7000) ///< Mux Mask
#define ADS1015_REG_CONFIG_MUX_DIFF_0_1                                        \
  (0x0000) ///< Differential P = AIN0, N = AIN1 (default)
#define ADS1015_REG_CONFIG_MUX_DIFF_0_3                                        \
  (0x1000) ///< Differential P = AIN0, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_1_3                                        \
  (0x2000) ///< Differential P = AIN1, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_2_3                                        \
  (0x3000) ///< Differential P = AIN2, N = AIN3
#define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000) ///< Single-ended AIN0
#define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000) ///< Single-ended AIN1
#define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000) ///< Single-ended AIN2
#define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000) ///< Single-ended AIN3

#define ADS1015_REG_CONFIG_PGA_MASK (0x0E00)   ///< PGA Mask
#define ADS1015_REG_CONFIG_PGA_6_144V (0x0000) ///< +/-6.144V range = Gain 2/3
#define ADS1015_REG_CONFIG_PGA_4_096V (0x0200) ///< +/-4.096V range = Gain 1
#define ADS1015_REG_CONFIG_PGA_2_048V                                          \
  (0x0400) ///< +/-2.048V range = Gain 2 (default)
#define ADS1015_REG_CONFIG_PGA_1_024V (0x0600) ///< +/-1.024V range = Gain 4
#define ADS1015_REG_CONFIG_PGA_0_512V (0x0800) ///< +/-0.512V range = Gain 8
#define ADS1015_REG_CONFIG_PGA_0_256V (0x0A00) ///< +/-0.256V range = Gain 16

#define ADS1015_REG_CONFIG_MODE_MASK (0x0100)   ///< Mode Mask
#define ADS1015_REG_CONFIG_MODE_CONTIN (0x0000) ///< Continuous conversion mode
#define ADS1015_REG_CONFIG_MODE_SINGLE                                         \
  (0x0100) ///< Power-down single-shot mode (default)

#define ADS1015_REG_CONFIG_DR_MASK (0x00E0)   ///< Data Rate Mask

#define ADS1015_REG_CONFIG_DR_128SPS (0x0000) ///< 128 samples per second
#define ADS1015_REG_CONFIG_DR_250SPS (0x0020) ///< 250 samples per second
#define ADS1015_REG_CONFIG_DR_490SPS (0x0040) ///< 490 samples per second
#define ADS1015_REG_CONFIG_DR_920SPS (0x0060) ///< 920 samples per second
#define ADS1015_REG_CONFIG_DR_1600SPS (0x0080) ///< 1600 samples per second (default)
#define ADS1015_REG_CONFIG_DR_2400SPS (0x00A0) ///< 2400 samples per second
#define ADS1015_REG_CONFIG_DR_3300SPS (0x00C0) ///< 3300 samples per second

#define ADS1115_REG_CONFIG_DR_8SPS (0x0000) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_16SPS (0x0020) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_32SPS (0x0040) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_64SPS (0x0060) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_128SPS (0x0080) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_250SPS (0x00A0) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_475SPS (0x00C0) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_860SPS (0x00E0) ///< 8 samples per second

#define ADS1015_REG_CONFIG_CMODE_MASK (0x0010) ///< CMode Mask
#define ADS1015_REG_CONFIG_CMODE_TRAD                                          \
  (0x0000) ///< Traditional comparator with hysteresis (default)
#define ADS1015_REG_CONFIG_CMODE_WINDOW (0x0010) ///< Window comparator

#define ADS1015_REG_CONFIG_CPOL_MASK (0x0008) ///< CPol Mask
#define ADS1015_REG_CONFIG_CPOL_ACTVLOW                                        \
  (0x0000) ///< ALERT/RDY pin is low when active (default)
#define ADS1015_REG_CONFIG_CPOL_ACTVHI                                         \
  (0x0008) ///< ALERT/RDY pin is high when active

#define ADS1015_REG_CONFIG_CLAT_MASK                                           \
  (0x0004) ///< Determines if ALERT/RDY pin latches once asserted
#define ADS1015_REG_CONFIG_CLAT_NONLAT                                         \
  (0x0000) ///< Non-latching comparator (default)
#define ADS1015_REG_CONFIG_CLAT_LATCH (0x0004) ///< Latching comparator

#define ADS1015_REG_CONFIG_CQUE_MASK (0x0003) ///< CQue Mask
#define ADS1015_REG_CONFIG_CQUE_1CONV                                          \
  (0x0000) ///< Assert ALERT/RDY after one conversions
#define ADS1015_REG_CONFIG_CQUE_2CONV                                          \
  (0x0001) ///< Assert ALERT/RDY after two conversions
#define ADS1015_REG_CONFIG_CQUE_4CONV                                          \
  (0x0002) ///< Assert ALERT/RDY after four conversions
#define ADS1015_REG_CONFIG_CQUE_NONE                                           \
  (0x0003) ///< Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

/** Gain settings */
typedef enum {
  GAIN_TWOTHIRDS = ADS1015_REG_CONFIG_PGA_6_144V,
  GAIN_ONE = ADS1015_REG_CONFIG_PGA_4_096V,
  GAIN_TWO = ADS1015_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR = ADS1015_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT = ADS1015_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN = ADS1015_REG_CONFIG_PGA_0_256V
} adsGain_t;

typedef enum{
  ADS1015_DATARATE_128SPS = ADS1015_REG_CONFIG_DR_128SPS,
  ADS1015_DATARATE_250SPS = ADS1015_REG_CONFIG_DR_250SPS,
  ADS1015_DATARATE_490SPS = ADS1015_REG_CONFIG_DR_490SPS,
  ADS1015_DATARATE_920SPS = ADS1015_REG_CONFIG_DR_920SPS,
  ADS1015_DATARATE_1600SPS = ADS1015_REG_CONFIG_DR_1600SPS,
  ADS1015_DATARATE_2400SPS = ADS1015_REG_CONFIG_DR_2400SPS,
  ADS1015_DATARATE_3300SPS = ADS1015_REG_CONFIG_DR_3300SPS,

  ADS1115_DATARATE_8SPS = ADS1115_REG_CONFIG_DR_8SPS,
  ADS1115_DATARATE_16SPS = ADS1115_REG_CONFIG_DR_16SPS,
  ADS1115_DATARATE_32SPS = ADS1115_REG_CONFIG_DR_32SPS,
  ADS1115_DATARATE_64SPS = ADS1115_REG_CONFIG_DR_64SPS,
  ADS1115_DATARATE_128SPS = ADS1115_REG_CONFIG_DR_128SPS,
  ADS1115_DATARATE_250SPS = ADS1115_REG_CONFIG_DR_250SPS,
  ADS1115_DATARATE_475SPS = ADS1115_REG_CONFIG_DR_475SPS,
  ADS1115_DATARATE_860SPS = ADS1115_REG_CONFIG_DR_860SPS
} ads1x15DataRate_t;

typedef enum{
  ADS1015,
  ADS1115
} ads1x15DeviceType_t;

typedef struct{
  ads1x15DeviceType_t deviceType;
  adsGain_t gain;
  ads1x15DataRate_t dataRate;
  uint8_t bitShift;
  uint8_t commsOk;
} ads1x15Settings_t;

void ADS1x15_SendRegs(I2C_HandleTypeDef* hi2c, uint16_t addr, ads1x15Settings_t *settings, uint8_t channel);
uint16_t ADS1x15_ReadADC(I2C_HandleTypeDef* hi2c, uint16_t addr, ads1x15Settings_t *settings);

#endif /* SRC_ADS1X15_ADS1X15_H_ */
