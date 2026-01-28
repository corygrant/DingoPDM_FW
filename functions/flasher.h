#pragma once

#include <cstdint>
#include "config.h"

extern float *pVarMap[PDM_VAR_MAP_SIZE];

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
    static void SetDefaultConfig(Config_Flasher *config);

    float fVal;

private:
    Config_Flasher* pConfig;
    
    float *pInput;

    uint32_t nTimeOff;
    uint32_t nTimeOn;
};