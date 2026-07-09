#include "hw_devices.h"
#include "port_pwm.h"

Profet pf[NUM_OUTPUTS] = {
    Profet(1, ProfetModel::BTS70012_1ESP, LINE_PF1_IN, LINE_PF1_DEN, LINE_UNUSED, AnalogChannel::IS1, &PWMD3, &pwm3Cfg, PwmChannel::Ch1),
    Profet(2, ProfetModel::BTS70012_1ESP, LINE_PF2_IN, LINE_PF2_DEN, LINE_UNUSED, AnalogChannel::IS2, &PWMD4, &pwm4Cfg, PwmChannel::Ch1),
    Profet(3, ProfetModel::BTS70012_1ESP, LINE_PF3_IN, LINE_PF3_DEN, LINE_UNUSED, AnalogChannel::IS3, &PWMD5, &pwm5Cfg, PwmChannel::Ch1),
    Profet(4, ProfetModel::BTS70012_1ESP, LINE_PF4_IN, LINE_PF4_DEN, LINE_UNUSED, AnalogChannel::IS4, &PWMD9, &pwm9Cfg, PwmChannel::Ch1)};


Digital_Input digIn[NUM_DIG_INPUTS] = {
    Digital_Input(LINE_DI1),
    Digital_Input(LINE_DI2)};    

Led statusLed = Led(LINE_LED_STATUS);
Led errorLed = Led(LINE_LED_ERROR);

MCP9808 tempSensor(I2CD1, MCP9808_I2CADDR_DEFAULT);
