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
    void CheckTimeout();
    bool GetEnable() { return pConfig->bEnabled; }
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);

    uint16_t nOutput;
    uint16_t nVal;

private:
    Config_CanInput* pConfig;
    uint16_t nData;

    Input input;

    uint32_t nLastRxTime;
};