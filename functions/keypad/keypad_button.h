#pragma once

#include <cstdint>
#include "input.h"
#include "enums.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_KeypadButton{
  bool bEnabled;
  InputMode eMode;
  uint8_t nColors[4];
  uint8_t nFaultColor;
  uint16_t nVars[4];
  uint16_t nFaultVar;
  bool bBlink[4];
  bool bFaultBlink;
  uint8_t nBlinkColors[4];
  uint8_t nFaultBlinkColor;
};

class KeypadButton;

// Function pointer types for brand-specific LED behavior
using UpdateButtonLedFn = void (*)(KeypadButton*);

class KeypadButton
{
public:
    KeypadButton() = default;

    void SetConfig(Config_KeypadButton *config)
    {
        pConfig = config;
        pLedVars[0] = pVarMap[config->nVars[0]];
        pLedVars[1] = pVarMap[config->nVars[1]];
        pLedVars[2] = pVarMap[config->nVars[2]];
        pLedVars[3] = pVarMap[config->nVars[3]];
        pFaultLedVar = pVarMap[config->nFaultVar];
    }

    Config_KeypadButton *pConfig = nullptr;

    float *pLedVars[4] = {};
    float *pFaultLedVar = nullptr;

    bool UpdateState(bool bNewVal);
    void UpdateLedState()
    {
        if (fnUpdateButtonLed)
            fnUpdateButtonLed(this);
    }

    void SetBrand(UpdateButtonLedFn updateFn)
    {
        fnUpdateButtonLed = updateFn;
    }

    //Blink Marine-specific LED state
    BlinkMarineButtonColor eLedOnColor = BlinkMarineButtonColor::Off;
    BlinkMarineButtonColor eLedBlinkColor = BlinkMarineButtonColor::Off;

    //Grayhill specific LED state
    bool bLed[3] = {false, false, false};

    Input input;
    bool bVal = false;    

private:
    UpdateButtonLedFn fnUpdateButtonLed = nullptr;
};
