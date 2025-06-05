#pragma once

#include <cstdint>
#include "config.h"

extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

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

    uint16_t nVal;

private:
    Config_Condition* pConfig;
    
    uint16_t *pInput;
};