#pragma once

#include "port.h"
#include "input.h"

struct Config_AnalogSwitch{
  bool bEnabled;
  InputMode eMode;
  bool bInvert;
  uint16_t nThreshold;
};

struct Config_RotarySwitch{
  bool bEnabled;
  bool bInvert;
  float fOffset;
  float fStep;
  float fMaxPos;
};

struct Config_AnalogInput{
  bool bEnabled;
  Config_AnalogSwitch stSwitch;
  Config_RotarySwitch stRotary;
};

class Analog_Input
{
public:
    Analog_Input(const AnalogChannel channel) 
                : m_channel(channel) 
    {};

    static const uint16_t nBaseIndex = 0x2200;

    void SetConfig(Config_AnalogInput *config)
    {
        pConfig = config;
    }

    void Update();

    float fVal;
    float fValMillivolts;
    float fRotaryPos;
    float fSwitchVal;

private:
    const AnalogChannel m_channel;

    Config_AnalogInput* pConfig;    

    Input input;

    void RotaryUpdate();
    void SwitchUpdate();
};