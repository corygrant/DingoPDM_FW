#pragma once

#include "port.h"
#include "config.h"
#include "hal.h"
#include "input.h"

class CanInput
{
public:
    CanInput() {
    };

    bool CheckMsg(CANRxFrame frame);

    void SetConfig(Config_CanInput* config) { pConfig = config; }
    void SetRxTimeout(uint32_t timeout) { nTimeout = timeout; }
    void CheckTimeout();

    uint16_t nVal;

private:
    Config_CanInput* pConfig;
    uint64_t nData;

    Input input;

    uint32_t nLastRxTime;
    uint32_t nTimeout = 2000; //ms
};