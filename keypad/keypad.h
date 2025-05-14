#pragma once

#include <cstdint>
#include "config.h"
#include "mailbox.h"
#include "dingopdm_config.h"
#include "keypad_button.h"

class Keypad
{
public: 
    Keypad() {};

    void SetConfig(Config_Keypad* config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pDimmingInput = &pVarMap[config->nDimmingVar][0];
        for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        {
            button[i].SetConfig(&pConfig->stButton[i], pVarMap);
        }
    };

    msg_t Init();

    void CheckTimeout();

    bool CheckMsg(CANRxFrame frame);

    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);

    CANTxFrame GetTxMsg(uint8_t nIndex);
    CANTxFrame GetStartMsg();

    uint16_t nVal[KEYPAD_MAX_BUTTONS];

protected:
    Config_Keypad* pConfig;

    uint32_t nLastRxTime;

    uint16_t *pDimmingInput;

    KeypadButton button[KEYPAD_MAX_BUTTONS];

    uint8_t nNumButtons;
    

    // Blink Marine specific
    CANTxFrame LedOnMsg();
    CANTxFrame LedBlinkMsg();
    CANTxFrame LedBrightnessMsg();
    CANTxFrame BacklightMsg();

    uint64_t BuildLedMsg(bool bBlink); 

    bool ColorToRed(BlinkMarineButtonColor eColor);
    bool ColorToGreen(BlinkMarineButtonColor eColor);
    bool ColorToBlue(BlinkMarineButtonColor eColor);

    uint8_t nNumDials;


    // Grayhill specific
    CANTxFrame IndicatorMsg();
    CANTxFrame BrightnessMsg();
};