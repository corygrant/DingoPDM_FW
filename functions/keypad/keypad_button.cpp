#include "keypad_button.h"
#include "dbc.h"

bool KeypadButton::Update(bool bNewVal)
{
    if (!pConfig->bEnabled)
        return false;

    bVal = input.Check(pConfig->eMode, false, bNewVal);

    return bVal;
}

// Set color based on var map values and fault state
void KeypadButton::UpdateLed()
{
    if (!pConfig->bEnabled)
        return;

    BlinkMarineButtonColor eColor = BlinkMarineButtonColor::Off;
    BlinkMarineButtonColor eBlinkColor = BlinkMarineButtonColor::Off;

    for (uint8_t i = 0; i < pConfig->nNumOfValColors; i++)
    {
        // Check if the value is equal to the index of the color
        // If so, set the color to the corresponding value color
        // Boolean values only check values 0 and 1
        // Integer values check values 0 to nNumOfValColors - 1
        if (static_cast<uint8_t>(*pLedVars[i]) == 1)
        {
            eColor = static_cast<BlinkMarineButtonColor>(pConfig->nValColors[i]);

            if (pConfig->bValBlinking[i])
            {
                if (eColor == BlinkMarineButtonColor::Off)
                    eBlinkColor = static_cast<BlinkMarineButtonColor>(pConfig->nValBlinkingColors[i]);
                else
                    eBlinkColor = static_cast<BlinkMarineButtonColor>(pConfig->nValBlinkingColors[i] ^ pConfig->nValColors[i]);
            }
        }
    }

    // Fault color takes precedence over value colors
    if (*pFaultLedVar == 1)
    {
        eColor = (BlinkMarineButtonColor)pConfig->nFaultColor;

        if(pConfig->bFaultBlinking)
        {
            if (eColor == BlinkMarineButtonColor::Off)
                eBlinkColor = (BlinkMarineButtonColor)pConfig->nFaultBlinkingColor;
            else
                eBlinkColor = (BlinkMarineButtonColor)(pConfig->nFaultBlinkingColor ^ pConfig->nFaultColor);
        }
    }

    eLedOnColor = eColor;
    eLedBlinkColor = eBlinkColor;
}

MsgCmdResult ButtonMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set keypad button settings
    // DLC 2 = Get keypad button settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 3);
        uint8_t nButtonIndex = Dbc::DecodeInt(rx->data8, 11, 5);

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 8)
            {
                conf->stKeypad[nIndex].stButton[nButtonIndex].bEnabled = Dbc::DecodeInt(rx->data8, 16, 1);
                conf->stKeypad[nIndex].stButton[nButtonIndex].eMode = static_cast<InputMode>(Dbc::DecodeInt(rx->data8, 18, 2));

                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[0] = Dbc::DecodeInt(rx->data8, 24, 8);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[1] = Dbc::DecodeInt(rx->data8, 32, 8);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[2] = Dbc::DecodeInt(rx->data8, 40, 8);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[3] = Dbc::DecodeInt(rx->data8, 48, 8);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultVar = Dbc::DecodeInt(rx->data8, 56, 8);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;
            for (int i = 0; i < 8; i++) tx->data8[i] = 0;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::KeypadButton) + 128;
            Dbc::EncodeInt(tx->data8, nIndex, 8, 3);
            Dbc::EncodeInt(tx->data8, nButtonIndex, 11, 5);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].bEnabled, 16, 1);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stKeypad[nIndex].stButton[nButtonIndex].eMode), 18, 2);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[0], 24, 8);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[1], 32, 8);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[2], 40, 8);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[3], 48, 8);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultVar, 56, 8);

            if(rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

MsgCmdResult ButtonLedMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set keypad button LED settings
    // DLC 2 = Get keypad button LED settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = Dbc::DecodeInt(rx->data8, 8, 3);
        uint8_t nButtonIndex = Dbc::DecodeInt(rx->data8, 11, 5);

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 8)
            {
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[0] = Dbc::DecodeInt(rx->data8, 16, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[1] = Dbc::DecodeInt(rx->data8, 20, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[2] = Dbc::DecodeInt(rx->data8, 24, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[3] = Dbc::DecodeInt(rx->data8, 28, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultColor = Dbc::DecodeInt(rx->data8, 32, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nNumOfValColors = Dbc::DecodeInt(rx->data8, 36, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[0] = Dbc::DecodeInt(rx->data8, 40, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[1] = Dbc::DecodeInt(rx->data8, 44, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[2] = Dbc::DecodeInt(rx->data8, 48, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[3] = Dbc::DecodeInt(rx->data8, 52, 4);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultBlinkingColor = Dbc::DecodeInt(rx->data8, 56, 4);

                conf->stKeypad[nIndex].stButton[nButtonIndex].bValBlinking[0] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[0] > 0;
                conf->stKeypad[nIndex].stButton[nButtonIndex].bValBlinking[1] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[1] > 0;
                conf->stKeypad[nIndex].stButton[nButtonIndex].bValBlinking[2] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[2] > 0;
                conf->stKeypad[nIndex].stButton[nButtonIndex].bValBlinking[3] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[3] > 0;
                conf->stKeypad[nIndex].stButton[nButtonIndex].bFaultBlinking = conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultBlinkingColor > 0;
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;
            for (int i = 0; i < 8; i++) tx->data8[i] = 0;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::KeypadButtonLed) + 128;
            Dbc::EncodeInt(tx->data8, nIndex, 8, 3);
            Dbc::EncodeInt(tx->data8, nButtonIndex, 11, 5);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[0], 16, 4);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[1], 20, 4);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[2], 24, 4);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[3], 28, 4);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultColor, 32, 4);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nNumOfValColors, 36, 4);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[0], 40, 4);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[1], 44, 4);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[2], 48, 4);
            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[3], 52, 4);

            Dbc::EncodeInt(tx->data8, conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultBlinkingColor, 56, 4);
            
            if(rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

MsgCmdResult KeypadButton::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    MsgCmd cmd = static_cast<MsgCmd>(rx->data8[0]);

    if(cmd == MsgCmd::KeypadButton)
        return ButtonMsg(conf, rx, tx);
    else if(cmd == MsgCmd::KeypadButtonLed)
        return ButtonLedMsg(conf, rx, tx);
    return MsgCmdResult::Invalid;
}

void KeypadButton::SetDefaultConfig(Config_KeypadButton *config)
{
    config->bEnabled = false;
    config->eMode = InputMode::Momentary;
    config->nNumOfValColors = 4;
    config->nValColors[0] = (uint8_t)BlinkMarineButtonColor::Off;
    config->nValColors[1] = (uint8_t)BlinkMarineButtonColor::Green;
    config->nValColors[2] = (uint8_t)BlinkMarineButtonColor::Violet;
    config->nValColors[3] = (uint8_t)BlinkMarineButtonColor::Blue;
    config->nFaultColor = (uint8_t)BlinkMarineButtonColor::Red;
    config->nValVars[0] = 0;
    config->nValVars[1] = 0;
    config->nValVars[2] = 0;
    config->nValVars[3] = 0;
    config->nFaultVar = 0;
    config->bValBlinking[0] = false;
    config->bValBlinking[1] = false;
    config->bValBlinking[2] = false;
    config->bValBlinking[3] = false;
    config->bFaultBlinking = false;
    config->nValBlinkingColors[0] = (uint8_t)BlinkMarineButtonColor::Blue;
    config->nValBlinkingColors[1] = (uint8_t)BlinkMarineButtonColor::Violet;
    config->nValBlinkingColors[2] = (uint8_t)BlinkMarineButtonColor::Green;
    config->nValBlinkingColors[3] = (uint8_t)BlinkMarineButtonColor::White;
    config->nFaultBlinkingColor = (uint8_t)BlinkMarineButtonColor::Orange;
}