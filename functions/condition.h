#pragma once

#include <cstdint>
#include "config.h"

extern float *pVarMap[PDM_VAR_MAP_SIZE];

class Condition
{
public:
    Condition() {
    };

    void SetConfig(Config_Condition* config)
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update();
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_Condition *config);

    float fVal;

private:
    Config_Condition* pConfig;
    
    float *pInput;
};