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
        pLedVars[0] = pVarMap[config->nValVars[0]];
        pLedVars[1] = pVarMap[config->nValVars[1]];
        pLedVars[2] = pVarMap[config->nValVars[2]];
        pLedVars[3] = pVarMap[config->nValVars[3]];
        pFaultLedVar = pVarMap[config->nFaultVar];
    }

    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    
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

    uint16_t *pLedVars[4];
    uint16_t *pFaultLedVar;

    bool bVal;
};