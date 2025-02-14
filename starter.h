#pragma once

#include <cstdint>
#include "config.h"

class Starter
{
public:
    Starter() {

    };

    void SetConfig(Config_Starter* config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update();
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);

    uint16_t nVal[PDM_NUM_OUTPUTS];

private:
    Config_Starter* pConfig;
    
    uint16_t *pInput;

    
};