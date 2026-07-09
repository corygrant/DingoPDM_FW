#include "starter.h"
#include "dbc.h"

void Starter::Update()
{
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
    {
        if (!pConfig->bEnabled)
            fVal[i] = 1;
        else
            fVal[i] = !(pConfig->bDisableOut[i] && *pInput);
    }
}