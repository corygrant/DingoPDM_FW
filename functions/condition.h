#pragma once

#include <cstdint>
#include "port.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_Condition{
  bool bEnabled;
  uint16_t nInput;
  Operator eOperator;
  float fArg;
};

class Condition
{
public:
    Condition() {
    };

    static const uint16_t nBaseIndex = 0x1500;

    void SetConfig(Config_Condition* config)
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update();

    float fVal;

private:
    Config_Condition* pConfig;
    
    float *pInput;
};