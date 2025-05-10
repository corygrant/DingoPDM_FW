#pragma once

#include <cstdint>
#include "input.h"

class KeypadButton
{
public:
    KeypadButton() {};

    void SetConfig(Config_KeypadButton *config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pValInput[0] = pVarMap[config->nValVars[0]];
        pValInput[1] = pVarMap[config->nValVars[1]];
        pValInput[2] = pVarMap[config->nValVars[2]];
        pValInput[3] = pVarMap[config->nValVars[3]];
        pFaultInput = pVarMap[config->nFaultVar];
    }

    bool Update(bool bNewVal);

    BlinkMarineButtonColor GetColor();

private:
    Config_KeypadButton *pConfig;
    Input input;

    uint16_t *pValInput[4];
    uint16_t *pFaultInput;

    bool bVal;
};