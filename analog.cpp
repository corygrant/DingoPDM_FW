#include "analog.h"
#include "ch.hpp"
#include "hal.h"
#include "mcu_utils.h"

adcsample_t adc1_samples[ADC1_NUM_CHANNELS] = {0};
//0 = OutIS1
//1 = OutIS2
//2 = OutIS3_4
//3 = OutIS5_6
//4 = OutIS7_8
//5 = BattVolt
//6 = TempSensor
//7 = VRefInt

static const ADCConversionGroup adc1_cfg = {
    .circular = true,
    .num_channels = ADC1_NUM_CHANNELS,
    .end_cb = NULL,
    .error_cb = NULL,
    .cr1 = 0,
    .cr2 = ADC_CR2_SWSTART | ADC_CR2_CONT,
    .smpr1 = ADC_SMPR1_SMP_AN12(ADC_SAMPLE_56)   |
             ADC_SMPR1_SMP_AN13(ADC_SAMPLE_56)   |
             ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_56) |
             ADC_SMPR1_SMP_VREF(ADC_SAMPLE_56),
    .smpr2 = ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56)    |
             ADC_SMPR2_SMP_AN1(ADC_SAMPLE_56)    |
             ADC_SMPR2_SMP_AN2(ADC_SAMPLE_56)    |
             ADC_SMPR2_SMP_AN3(ADC_SAMPLE_56),
    .htr = 0,                              
    .ltr = 0, 
    .sqr1 = 0,
    .sqr2 = ADC_SQR2_SQ7_N(ADC_CHANNEL_SENSOR) |
            ADC_SQR2_SQ8_N(ADC_CHANNEL_VREFINT),
    .sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)  |
            ADC_SQR3_SQ2_N(ADC_CHANNEL_IN12) |
            ADC_SQR3_SQ3_N(ADC_CHANNEL_IN13) |
            ADC_SQR3_SQ4_N(ADC_CHANNEL_IN1)  |
            ADC_SQR3_SQ5_N(ADC_CHANNEL_IN2)  |
            ADC_SQR3_SQ6_N(ADC_CHANNEL_IN3)                   
    };

void InitAdc()
{
    adcStart(&ADCD1, NULL);
    adcSTM32EnableTSVREFE(); //Enable temp sensor and vref

    //Need to continuous conversion to read both channels of the BTS7008-2EPA
    //Requires 2 channels to be read with a 100us delay between them
    //Profet DSEL pin toggled in profet.cpp
    adcStartConversion(&ADCD1, &adc1_cfg, adc1_samples, ADC1_BUF_DEPTH);
}

void DeInitAdc()
{
    adcStopConversion(&ADCD1);
    adcStop(&ADCD1);
}

uint16_t GetAdcRaw(AnalogChannel channel)
{
    switch (channel)
    {
    case AnalogChannel::IS1:
        return adc1_samples[0];
    case AnalogChannel::IS2:
        return adc1_samples[1];
    case AnalogChannel::IS3_4:
        return adc1_samples[2];
    case AnalogChannel::IS5_6:
        return adc1_samples[3];
    case AnalogChannel::IS7_8:
        return adc1_samples[4];
    case AnalogChannel::BattVolt:
        return adc1_samples[5];
    case AnalogChannel::TempSensor:
        return adc1_samples[6];
    case AnalogChannel::VRefInt:
        return adc1_samples[7];
    default:
        return 0; // Invalid channel
    }
}

float GetBattVolt()
{
    // MCU vRef = 3.3v
    // 4095 counts full scale
    float mcuVolts = (GetVDDA() / 4095) * GetAdcRaw(AnalogChannel::BattVolt);

    const float rUpper = 47000;
    const float rLower = 4700;

    return mcuVolts * ((rUpper + rLower) / rLower);
}

float GetTemperature()
{
    return (30.0 + ((float)(GetAdcRaw(AnalogChannel::TempSensor) - STM32_TEMP_3V3_30C) / (STM32_TEMP_3V3_110C - STM32_TEMP_3V3_30C)) * (110.0 - 30.0));
}

float GetVDDA()
{
    return ((float)STM32_VREF_INT_CAL / (float)GetAdcRaw(AnalogChannel::VRefInt)) * 3.3;
}