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