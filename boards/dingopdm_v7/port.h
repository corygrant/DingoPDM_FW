#pragma once

#include "hal.h"

#define PDM_TYPE 0 //0 = PDM, 1 = PDM-MAX 

#define PDM_NUM_OUTPUTS 8
#define PDM_NUM_INPUTS 2
#define PDM_NUM_VIRT_INPUTS 16
#define PDM_NUM_CAN_INPUTS 32
#define PDM_NUM_FLASHERS 4
#define PDM_NUM_WIPER_INTER_DELAYS 6
#define PDM_NUM_WIPER_SPEED_MAP 8

#define PDM_VAR_MAP_SIZE 66

#define PDM_NUM_TX_MSGS 6

#define STM32_TEMP_3V3_30C *((uint16_t *)0x1FFF7A2C)
#define STM32_TEMP_3V3_110C *((uint16_t *)0x1FFF7A2E)

#define STM32_VREF_INT_CAL *((uint16_t *)0x1FFF7A2A)

#define ADC1_NUM_CHANNELS 8
#define ADC1_BUF_DEPTH 1

#define BTS7002_1EPP_KILIS 229421
#define BTS7008_2EPA_KILIS 59481

enum class AnalogChannel
{
    IS1 = 0,
    IS2,
    IS3_4,
    IS5_6,
    IS7_8,
    BattVolt,
    TempSensor,
    VRefInt
};

enum class DigitalChannel
{
    In1,
    In2
};

enum class LedType
{
    Status,
    Error
};

const CANConfig &GetCanConfig();

const I2CConfig i2cConfig = {
    OPMODE_I2C,
    400000,
    FAST_DUTY_CYCLE_2,
};