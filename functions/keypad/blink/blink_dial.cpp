#include "blink_dial.h"
#include "dbc.h"

void BlinkDial::CheckMsg(uint64_t data)
{
    nCounter = data & 0x7F;
    nCounterMax = (data & 0xFF000000) >> 24;
}

void BlinkDial::Update()
{

    uint8_t nLedsToLight = (nCounter * 16 + nCounterMax - 1) / nCounterMax; // Round up division
    for (uint8_t i = 0; i < 16; i++)
    {
        uint8_t nLedIndex = (pConfig->nLedOffset + i) % 16; // Wrap around
        bLeds[nLedIndex] = (i < nLedsToLight);
    }

}