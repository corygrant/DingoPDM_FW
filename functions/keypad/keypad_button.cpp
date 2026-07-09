#include "keypad_button.h"

bool KeypadButton::UpdateState(bool bNewVal)
{
    if (!pConfig->bEnabled)
    {
        bVal = false;
        return false;
    }
    
    bVal = input.Check(pConfig->eMode, false, bNewVal);

    return bVal;
}
