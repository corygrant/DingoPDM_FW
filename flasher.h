#pragma once

#include <cstdint>
#include "config.h"

class Flasher
{
public:
    Flasher() {

    };

    void Update(uint32_t timeNow);

    void SetConfig(Config_Flasher* config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    uint16_t nVal;

private:
    Config_Flasher* pConfig;
    
    uint16_t *pInput;

    uint32_t nTimeOff;
    uint32_t nTimeOn;
};