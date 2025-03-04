#pragma once

#include <cstdint>
#include "config.h"

class Condition
{
public:
    Condition() {
    };

    void SetConfig(Config_Condition* config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update();
    bool GetEnable() { return pConfig->bEnabled; }
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);

    uint16_t nVal;

private:
    Config_Condition* pConfig;
    
    uint16_t *pInput;
};