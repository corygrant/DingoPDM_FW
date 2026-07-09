#pragma once

#include <cstdint>
#include "port.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_Flasher{
  bool bEnabled;
  uint16_t nInput;
  uint16_t nFlashOnTime; //ms
  uint16_t nFlashOffTime; //ms
  bool bSingleCycle;
};

class Flasher
{
public:
    Flasher() {

    };

    static const uint16_t nBaseIndex = 0x1700;

    void SetConfig(Config_Flasher* config)
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update(uint32_t timeNow);

    float fVal;

private:
    Config_Flasher* pConfig;
    
    float *pInput;

    uint32_t nTimeOff;
    uint32_t nTimeOn;
};