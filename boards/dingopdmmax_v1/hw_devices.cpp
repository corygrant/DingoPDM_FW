#include "hw_devices.h"

#if PDM_TYPE == 1
Profet pf[PDM_NUM_OUTPUTS] = {
    Profet(1, ProfetModel::BTS70012_1ESP, LINE_PF1_IN, LINE_PF1_DEN, LINE_UNUSED, AnalogChannel::IS1),
    Profet(2, ProfetModel::BTS70012_1ESP, LINE_PF2_IN, LINE_PF2_DEN, LINE_UNUSED, AnalogChannel::IS2),
    Profet(3, ProfetModel::BTS70012_1ESP, LINE_PF3_IN, LINE_PF3_DEN, LINE_UNUSED, AnalogChannel::IS3),
    Profet(4, ProfetModel::BTS70012_1ESP, LINE_PF4_IN, LINE_PF4_DEN, LINE_UNUSED, AnalogChannel::IS4)};


Digital in[PDM_NUM_INPUTS] = {
    Digital(LINE_DI1),
    Digital(LINE_DI2)};    

Led statusLed = Led(LedType::Status);
Led errorLed = Led(LedType::Error);

MCP9808 tempSensor(I2CD1, MCP9808_I2CADDR_DEFAULT);

#endif