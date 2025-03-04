#include "port.h"
#include "mcu_utils.h"

static const CANConfig canConfig1000 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=2, Seq 1=15 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_SJW(0) | CAN_BTR_BRP(1)  | CAN_BTR_TS1(14) | CAN_BTR_TS2(1),
};

static const CANConfig canConfig500 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=4, Seq 1=15 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_SJW(0) | CAN_BTR_BRP(3)  | CAN_BTR_TS1(14) | CAN_BTR_TS2(1),
};

static const CANConfig canConfig250 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=8, Seq 1=15 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_SJW(0) | CAN_BTR_BRP(7)  | CAN_BTR_TS1(14) | CAN_BTR_TS2(1),
};

static const CANConfig canConfig125 =
{
    CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_TXFP,
    /*
     For 36MHz http://www.bittiming.can-wiki.info/ gives us Pre-scaler=16, Seq 1=15 and Seq 2=2. Subtract '1' for register values
    */
    CAN_BTR_SJW(0) | CAN_BTR_BRP(15)  | CAN_BTR_TS1(14) | CAN_BTR_TS2(1),
};

const CANConfig& GetCanConfig(CanBitrate bitrate) {
    switch(bitrate) {
        case CanBitrate::Bitrate_1000K:
            return canConfig1000;
        case CanBitrate::Bitrate_500K:
            return canConfig500;
        case CanBitrate::Bitrate_250K:
            return canConfig250;
        case CanBitrate::Bitrate_125K:
            return canConfig125;
        default:
            return canConfig500;
    }
    return canConfig500;
}

adcsample_t adc1_samples[ADC1_NUM_CHANNELS] = {0};
//0 = OutIS1 - ADC1_IN0
//1 = OutIS2 - ADC1_IN12
//2 = OutIS3_4 - ADC1_IN13
//3 = OutIS5_6 - ADC1_IN1
//4 = OutIS7_8 - ADC1_IN2
//5 = BattVolt - ADC1_IN3
//6 = TempSensor
//7 = VRefInt

static const ADCConversionGroup adc1_cfg = {
    .circular = true,
    .num_channels = ADC1_NUM_CHANNELS,
    .end_cb = NULL,
    .error_cb = NULL,
    .cr1 = 0,
    .cr2 = ADC_CR2_SWSTART | ADC_CR2_CONT,
    .smpr1 = ADC_SMPR1_SMP_AN12(ADC_SAMPLE_3)   |
             ADC_SMPR1_SMP_AN13(ADC_SAMPLE_3)   |
             ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_3) |
             ADC_SMPR1_SMP_VREF(ADC_SAMPLE_3),
    .smpr2 = ADC_SMPR2_SMP_AN0(ADC_SAMPLE_3)    |
             ADC_SMPR2_SMP_AN1(ADC_SAMPLE_3)    |
             ADC_SMPR2_SMP_AN2(ADC_SAMPLE_3)    |
             ADC_SMPR2_SMP_AN3(ADC_SAMPLE_3),
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

msg_t InitAdc()
{
    msg_t ret;
    ret = adcStart(&ADCD1, NULL);
    if(ret != HAL_RET_SUCCESS)
        return ret;

    adcSTM32EnableTSVREFE(); //Enable temp sensor and vref

    //Need to continuous conversion to read both channels of the BTS7008-2EPA
    //Requires 2 channels to be read with a 100us delay between them
    //Profet DSEL pin toggled in profet.cpp
    adcStartConversion(&ADCD1, &adc1_cfg, adc1_samples, ADC1_BUF_DEPTH);

    return HAL_RET_SUCCESS;
}

void DeInitAdc()
{
    adcStopConversion(&ADCD1);
    adcStop(&ADCD1);
}

uint16_t GetAdcRaw(AnalogChannel channel)
{
    return adc1_samples[static_cast<uint8_t>(channel)];
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