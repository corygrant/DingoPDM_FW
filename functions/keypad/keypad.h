#pragma once

#include <cstdint>
#include "mailbox.h"
#include "device_config.h"
#include "keypad_button.h"
#include "keypad_dial.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_Keypad{
  bool bEnabled;
  uint8_t nNodeId;
  bool bTimeoutEnabled;
  uint16_t nTimeout; //ms
  KeypadModel eModel;
  uint8_t nBacklightBrightness;
  uint8_t nDimBacklightBrightness;
  uint8_t nBacklightColor;
  uint16_t nDimmingVar;
  uint8_t nButtonBrightness;
  uint8_t nDimButtonBrightness;
  Config_KeypadButton stButton[KEYPAD_MAX_BUTTONS];
  Config_KeypadDial stDial[KEYPAD_MAX_DIALS];
};

class Keypad;  // Forward declaration

// Function pointer types for brand-specific behavior
using KeypadCheckMsgFn = bool (*)(Keypad*, CANRxFrame);
using KeypadGetTxMsgFn = CANTxFrame (*)(Keypad*, uint8_t);
using KeypadGetStartMsgFn = CANTxFrame (*)(Keypad*);
using KeypadSetModelFn = void (*)(Keypad*);

class Keypad
{
public:
    Keypad() {
    };

    void SetConfig(Config_Keypad* config);

    static const uint16_t nBaseIndex = 0x3000;

    static msg_t InitThread(Keypad *keypads);

    void Update();
    bool CheckMsg(CANRxFrame frame);

    CANTxFrame GetTxMsg(uint8_t nIndex);
    CANTxFrame GetStartMsg();
    uint8_t GetNumTxMsgs() const { return nNumTxMsgs; }

    bool IsEnabled() const { return pConfig->bEnabled; }

    // Var map data
    float fButtonVal[KEYPAD_MAX_BUTTONS];
    float fDialVal[KEYPAD_MAX_DIALS];
    float fAnalogVal[KEYPAD_MAX_ANALOG_INPUTS];

    Config_Keypad* pConfig = nullptr;
    float* pDimmingInput = nullptr;

    uint8_t nBacklightBrightness = 0;
    uint8_t nDimBacklightBrightness = 0;
    uint8_t nIndicatorBrightness = 0;

    KeypadButton button[KEYPAD_MAX_BUTTONS];
    KeypadDial dial[KEYPAD_MAX_DIALS];
    uint8_t nNumButtons = 0;
    uint8_t nNumDials = 0;
    uint8_t numAnalogInputs = 0;

    uint32_t nLastRxTime = 0;
    bool bStartMsgSent = false;

private:
    KeypadModel eLastModel = KeypadModel::Blink2Key;
    uint8_t nNumTxMsgs = 0;

    void CheckTimeout();

    // Function pointers - set by SetConfig based on brand
    KeypadCheckMsgFn fnCheckMsg = nullptr;
    KeypadGetTxMsgFn fnGetTxMsg = nullptr;
    KeypadGetStartMsgFn fnGetStartMsg = nullptr;
    KeypadSetModelFn fnSetModel = nullptr;
};
