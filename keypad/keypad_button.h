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
        pLedVar = pVarMap[config->nVar];
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

    uint16_t *pLedVar;
    uint16_t *pFaultLedVar;

    bool bVal;
};