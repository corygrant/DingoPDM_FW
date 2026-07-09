#include "blink_button.h"
#include "keypad_button.h"

void UpdateButtonLedBlinkMarine(KeypadButton* btn)
{
    if (!btn->pConfig->bEnabled)
        return;

    BlinkMarineButtonColor eColor = BlinkMarineButtonColor::Off;
    BlinkMarineButtonColor eBlinkColor = BlinkMarineButtonColor::Off;

    for (uint8_t i = 0; i < 4; i++)
    {
        // Check if the value is equal to the index of the color
        // If so, set the color to the corresponding value color
        // Boolean values only check values 0 and 1
        // Integer values check values 0 to nNumOfValColors - 1
        if (static_cast<uint8_t>(*btn->pLedVars[i]) == 1)
        {
            eColor = static_cast<BlinkMarineButtonColor>(btn->pConfig->nColors[i]);

            if (btn->pConfig->bBlink[i])
            {
                if (eColor == BlinkMarineButtonColor::Off)
                    eBlinkColor = static_cast<BlinkMarineButtonColor>(btn->pConfig->nBlinkColors[i]);
                else
                    eBlinkColor = static_cast<BlinkMarineButtonColor>(btn->pConfig->nBlinkColors[i] ^ btn->pConfig->nColors[i]);
            }
        }
    }

    // Fault color takes precedence over value colors
    if (*btn->pFaultLedVar == 1)
    {
        eColor = (BlinkMarineButtonColor)btn->pConfig->nFaultColor;

        if(btn->pConfig->bFaultBlink)
        {
            if (eColor == BlinkMarineButtonColor::Off)
                eBlinkColor = (BlinkMarineButtonColor)btn->pConfig->nFaultBlinkColor;
            else
                eBlinkColor = (BlinkMarineButtonColor)(btn->pConfig->nFaultBlinkColor ^ btn->pConfig->nFaultColor);
        }
    }

    btn->eLedOnColor = eColor;
    btn->eLedBlinkColor = eBlinkColor;
}
