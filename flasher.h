#pragma once

#include <cstdint>
#include "config.h"

extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

class Flasher
{
public:
    Flasher() {

    };

    void SetConfig(Config_Flasher* config)
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update(uint32_t timeNow);
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);

    uint16_t nVal;

private:
    Config_Flasher* pConfig;
    
    uint16_t *pInput;

    uint32_t nTimeOff;
    uint32_t nTimeOn;
};