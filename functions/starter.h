#pragma once

#include <cstdint>
#include "config.h"

extern float *pVarMap[PDM_VAR_MAP_SIZE];

class Starter
{
public:
    Starter() {

    };

    void SetConfig(Config_Starter* config)
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update();
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_Starter *config);

    float fVal[PDM_NUM_OUTPUTS];

private:
    Config_Starter* pConfig;
    
    float *pInput;

    
};