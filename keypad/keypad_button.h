#pragma once

#include <cstdint>
#include "input.h"

extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

class KeypadButton
{
public:
    KeypadButton() {};

    void SetConfig(Config_KeypadButton *config)
    {
        pConfig = config;
        pValInput[0] = pVarMap[config->nValVars[0]];
        pValInput[1] = pVarMap[config->nValVars[1]];
        pValInput[2] = pVarMap[config->nValVars[2]];
        pValInput[3] = pVarMap[config->nValVars[3]];
        pFaultInput = pVarMap[config->nFaultVar];
    }

    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_KeypadButton *config);
    
    bool Update(bool bNewVal);

    void UpdateLed();

    //Bit 0 = Red
    //Bit 1 = Green
    //Bit 2 = Blue
    BlinkMarineButtonColor eLedOnColor;
    BlinkMarineButtonColor eLedBlinkColor;

private:
    Config_KeypadButton *pConfig;
    Input input;

    uint16_t *pValInput[4];
    uint16_t *pFaultInput;

    bool bVal;
};