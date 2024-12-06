#include "flasher.h"

void Flasher::Update(uint32_t nTimeNow)
{
    if (!pConfig->bEnabled)
    {
        nVal = 0;
        return;
    }

    if (!*pInput)
    {
        nVal = 0;
        return;
    }
    
    if ((nVal == 0) && ((nTimeNow - nTimeOff) > pConfig->nFlashOffTime))
    {
        nVal = 1;
        nTimeOn = nTimeNow;
    }
    if ((nVal == 1) && ((nTimeNow - nTimeOn) > pConfig->nFlashOnTime))
    {
        nVal = 0;
        nTimeOff = nTimeNow;
    }
}