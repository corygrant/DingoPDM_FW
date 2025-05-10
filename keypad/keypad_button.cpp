#include "keypad_button.h"

bool KeypadButton::Update(bool bNewVal)
{
    if (!pConfig->bEnabled)
        return false;

    bVal = input.Check(pConfig->eMode, false, bNewVal);

    return bVal;
}

// Set color based on button state and fault state
BlinkMarineButtonColor KeypadButton::GetColor()
{
    if (!pConfig->bEnabled)
        return BlinkMarineButtonColor::BTN_OFF;

    BlinkMarineButtonColor eColor = BlinkMarineButtonColor::BTN_OFF;

    for (uint8_t i = 0; i < 4; i++)
    {
        if (*pValInput[i] == i)
            eColor = (BlinkMarineButtonColor)pConfig->nValColors[i];
    }

    if (*pFaultInput)
        eColor = (BlinkMarineButtonColor)pConfig->nFaultColor;

    return eColor;
}