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
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_CanInput *config);

    float fOutput;
    float fVal;

private:
    Config_CanInput* pConfig;

    Input input;

    uint32_t nLastRxTime;
};