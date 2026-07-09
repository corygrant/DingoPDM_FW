#pragma once

#include <cstdint>
#include "port.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_Counter{
  bool bEnabled;
  uint16_t nIncInput;
  uint16_t nDecInput;
  uint16_t nResetInput;
  uint8_t nMinCount;
  uint8_t nMaxCount;
  InputEdge eIncEdge;
  InputEdge eDecEdge;
  InputEdge eResetEdge;
  bool bWrapAround;
  bool bHoldToReset;
  uint16_t nResetTime; //ms
};

class Counter
{
public:
    Counter() {

    };

    static const uint16_t nBaseIndex = 0x1600;

    void SetConfig(Config_Counter* config)
    {
        pConfig = config;
        pIncInput = pVarMap[config->nIncInput];
        pDecInput = pVarMap[config->nDecInput];
        pResetInput = pVarMap[config->nResetInput];
    }

    void Update();

    float fVal;

private:
    Config_Counter* pConfig;
    
    float *pIncInput;
    float *pDecInput;
    float *pResetInput;

    bool bLastInc;
    bool bLastDec;
    bool bLastReset;
    
    uint32_t nLastIncTime;
    uint32_t nLastDecTime;
};
