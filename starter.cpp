#include "starter.h"

void Starter::Update()
{
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        if (!pConfig->bEnabled)
            nVal[i] = 1;
        else
            nVal[i] = !(pConfig->bDisableOut[i] && *pInput);
    }
}