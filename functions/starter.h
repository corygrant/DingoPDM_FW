#pragma once

#include <cstdint>
#include "port.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_Starter{
  bool bEnabled;
  uint16_t nInput;
  bool bDisableOut[NUM_OUTPUTS];
};

class Starter
{
public:
    Starter() {

    };

    static const uint16_t nBaseIndex = 0x1800;

    void SetConfig(Config_Starter* config)
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update();

    float fVal[NUM_OUTPUTS];

private:
    Config_Starter* pConfig;
    
    float *pInput;

    
};