#include "grayhill_button.h"
#include "keypad_button.h"

void UpdateButtonLedGrayhill(KeypadButton* btn)
{
    if (!btn->pConfig->bEnabled)
        return;

    for (uint8_t i = 0; i < 4; i++)
    {
        if (btn->pLedVars[i])
        {
            btn->bLed[0] = (btn->pConfig->nColors[i] & 0x01) > 0; // Bit 0
            btn->bLed[1] = (btn->pConfig->nColors[i] & 0x02) > 0; // Bit 1
            btn->bLed[2] = (btn->pConfig->nColors[i] & 0x04) > 0; // Bit 2
        }
    }

    // Fault LEDs takes precedence over value LEDs
    if (btn->pFaultLedVar)
    {
        btn->bLed[0] = (btn->pConfig->nFaultColor & 0x01) > 0; // Bit 0
        btn->bLed[1] = (btn->pConfig->nFaultColor & 0x02) > 0; // Bit 1
        btn->bLed[2] = (btn->pConfig->nFaultColor & 0x04) > 0; // Bit 2
    }
}