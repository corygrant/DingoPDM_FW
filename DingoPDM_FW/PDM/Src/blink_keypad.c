#include "blink_keypad.h"

void BlinkMarineKeypad_Init(BlinkMarineModel_t eModel, BlinkMarineKeypad_t *stKeypad){

    switch (eModel)
    {
    case PKP_1600_SI:
        stKeypad->nNumKeys = 6;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2200_SI:
        stKeypad->nNumKeys = 4;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2300_SI:
        stKeypad->nNumKeys = 6;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2400_SI:
        stKeypad->nNumKeys = 8;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2500_SI:
        stKeypad->nNumKeys = 10;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2600_SI:
        stKeypad->nNumKeys = 12;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_3500_SI:
        stKeypad->nNumKeys = 15;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_3500_SI_MT:
        stKeypad->nNumKeys = 6;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        stKeypad->bSkipButtonLed[0] = true; //Used by encoder 1
        stKeypad->bSkipButtonLed[10] = true; //Used by encoder 2
        break;

    case PKP_1200_LI:
        stKeypad->nNumKeys = 2;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_1500_LI:
        stKeypad->nNumKeys = 5;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2200_LI:
        stKeypad->nNumKeys = 4;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2400_LI:
        stKeypad->nNumKeys = 8;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2300_SI_FR:
        stKeypad->nNumKeys = 6;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2400_SI_FR:
        stKeypad->nNumKeys = 8;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;

    case PKP_2600_SI_FR:
        stKeypad->nNumKeys = 12;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;
    
    default:
        stKeypad->nNumKeys = 0;
        stKeypad->nNumKnobs = 0;
        stKeypad->nNumAnalogInputs = 0;
        break;
    }
}

bool BlinkMarineKeypad_IsMsg(uint16_t nId, BlinkMarineKeypad_t *stKeypad){

    return ((nId == stKeypad->nBaseId + 0x700) || (nId == stKeypad->nBaseId + 0x180) || (nId == stKeypad->nBaseId + 0x280) || (nId == stKeypad->nBaseId + 0x380) || 
            (nId == stKeypad->nBaseId + 0x480) || (nId == stKeypad->nBaseId + 0x200) || (nId == stKeypad->nBaseId + 0x300) || (nId == stKeypad->nBaseId + 0x400) || 
            (nId == stKeypad->nBaseId + 0x500) || (nId == stKeypad->nBaseId + 0x600) || (nId == stKeypad->nBaseId + 0x580) );
}

void BlinkMarineKeypad_Read(BlinkMarineKeypad_t *stKeypad){

}

void BlinkMarineKeypad_Write(BlinkMarineKeypad_t *stKeypad){
    //PDOs
    
}