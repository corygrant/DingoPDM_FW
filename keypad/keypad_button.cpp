#include "keypad_button.h"

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
        if (*pValInput[i] == i)
        {
            eColor = (BlinkMarineButtonColor)pConfig->nValColors[i];

            if(pConfig->bValBlinking[i])
                eBlinkColor = (BlinkMarineButtonColor)pConfig->nValBlinkingColors[i];
        }
    }

    // Fault color takes precedence over value colors
    if (*pFaultInput)
    {
        eColor = (BlinkMarineButtonColor)pConfig->nFaultColor;

        if(pConfig->bFaultBlinking)
            eBlinkColor = (BlinkMarineButtonColor)pConfig->nFaultBlinkingColor;
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
        uint8_t nIndex = (rx->data8[1] & 0x07);
        uint8_t nButtonIndex = (rx->data8[1] & 0xF8) >> 3;

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 8)
            {
                conf->stKeypad[nIndex].stButton[nButtonIndex].bEnabled = (rx->data8[2] & 0x01);
                conf->stKeypad[nIndex].stButton[nButtonIndex].bHasDial = (rx->data8[2] & 0x02) >> 1;
                conf->stKeypad[nIndex].stButton[nButtonIndex].eMode = static_cast<InputMode>((rx->data8[2] & 0x0C) >> 2);

                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[0] = rx->data8[3];
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[1] = rx->data8[4];
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[2] = rx->data8[5];
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[3] = rx->data8[6];
                conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultVar = rx->data8[7];
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::KeypadButton) + 128;
            tx->data8[1] = (nIndex & 0x07) + ((nButtonIndex & 0x1F) << 3);

            tx->data8[2] = (conf->stKeypad[nIndex].stButton[nButtonIndex].bEnabled & 0x01) +
                           ((conf->stKeypad[nIndex].stButton[nButtonIndex].bHasDial & 0x01) << 1) +
                           ((static_cast<uint8_t>(conf->stKeypad[nIndex].stButton[nButtonIndex].eMode) & 0x03) << 2);

            tx->data8[3] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[0];
            tx->data8[4] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[1];
            tx->data8[5] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[2];
            tx->data8[6] = conf->stKeypad[nIndex].stButton[nButtonIndex].nValVars[3];
            tx->data8[7] = conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultVar;

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
        uint8_t nIndex = (rx->data8[1] & 0x07);
        uint8_t nButtonIndex = (rx->data8[1] & 0xF8) >> 3;

        if (nIndex < PDM_NUM_KEYPADS)
        {
            if (rx->DLC == 8)
            {
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[0] = (rx->data8[2] & 0x0F);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[1] = (rx->data8[2] & 0xF0) >> 4;
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[2] = (rx->data8[3] & 0x0F);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[3] = (rx->data8[3] & 0xF0) >> 4;
                conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultColor = (rx->data8[4] & 0x0F);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nNumOfValColors = (rx->data8[4] & 0xF0) >> 4;
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[0] = (rx->data8[5] & 0x0F);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[1] = (rx->data8[5] & 0xF0) >> 4;
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[2] = (rx->data8[6] & 0x0F);
                conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[3] = (rx->data8[6] & 0xF0) >> 4;
                conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultBlinkingColor = (rx->data8[7] & 0x0F);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;
            tx->data8[0] = static_cast<uint8_t>(MsgCmd::KeypadButton) + 128;
            tx->data8[1] = (nIndex & 0x07) + ((nButtonIndex & 0x1F) << 3);

            tx->data8[2] = (conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[0] & 0x0F) +
                           ((conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[1] & 0x0F) << 4);
                           
            tx->data8[3] = (conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[2] & 0x0F) +
                           ((conf->stKeypad[nIndex].stButton[nButtonIndex].nValColors[3] & 0x0F) << 4);

            tx->data8[4] = (conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultColor & 0x0F) +
                           ((conf->stKeypad[nIndex].stButton[nButtonIndex].nNumOfValColors & 0x0F) << 4);

            tx->data8[5] = (conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[0] & 0x0F) +
                           ((conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[1] & 0x0F) << 4);

            tx->data8[6] = (conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[2] & 0x0F) +
                           ((conf->stKeypad[nIndex].stButton[nButtonIndex].nValBlinkingColors[3] & 0x0F) << 4);

            tx->data8[7] = (conf->stKeypad[nIndex].stButton[nButtonIndex].nFaultBlinkingColor & 0x0F);
            
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