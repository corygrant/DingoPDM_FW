#pragma once

#include <cstdint>
#include "config.h"
#include "mailbox.h"
#include "dingopdm_config.h"
#include "keypad_button.h"
#include "keypad_dial.h"

extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

class Keypad
{
public: 
    Keypad() {};

    void SetConfig(Config_Keypad* config)
    {
        pConfig = config;
        pDimmingInput = &pVarMap[config->nDimmingVar][0];
        for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        {
            button[i].SetConfig(&pConfig->stButton[i]);
        }
        for (uint8_t i = 0; i < KEYPAD_MAX_DIALS; i++)
        {
            dial[i].SetConfig(&pConfig->stDial[i]);
        }
    };

    msg_t Init(uint8_t index = 0);

    void CheckTimeout();

    bool CheckMsg(CANRxFrame frame);

    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_Keypad *config);

    CANTxFrame GetTxMsg(uint8_t nIndex);
    CANTxFrame GetStartMsg();

    uint16_t nVal[KEYPAD_MAX_BUTTONS];
    uint16_t nDialVal[KEYPAD_MAX_DIALS];

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

    KeypadDial dial[KEYPAD_MAX_DIALS];

    uint8_t nNumDials;


    // Grayhill specific
    CANTxFrame IndicatorMsg();
    CANTxFrame BrightnessMsg();
};