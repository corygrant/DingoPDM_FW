#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"

struct Config_KeypadDial{
  bool bEnabled;
  uint8_t nMinCount;
  uint8_t nMaxCount;
  uint8_t nLedOffset;
};

class BlinkDial
{
public:
    BlinkDial() {};

    void SetConfig(Config_KeypadDial *config)
    {
        pConfig = config;
    }
    
    void CheckMsg(uint64_t data);

    void Update();

    bool GetLedState(uint8_t nIndex)
    {
        if (nIndex >= 16)
            return false;

        return bLeds[nIndex];
    }

private:
    Config_KeypadDial *pConfig;

    uint8_t nCounter;
    uint8_t nCounterMax;

    bool bLeds[16];
};