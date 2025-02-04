#include "digital.h"
#include "input.h"

void Digital::Update()
{
    if(!pConfig->bEnabled)
    {
        nVal = 0;
        return;
    }

    bool bIn;

    bIn = palReadLine(m_line);

    // Debounce input
    if (bIn != bLast)
    {
        nLastTrigTime = SYS_TIME;
        bCheck = true;
    }

    bLast = bIn;

    if ((bCheck && ((SYS_TIME - nLastTrigTime) > pConfig->nDebounceTime)) || (!bInit))
    {
        bCheck = false;
        nVal = input.Check(pConfig->eMode, pConfig->bInvert, bIn);
    }

    bInit = true;
}

void Digital::SetPull(InputPull pull)
{
    switch (pull)
    {
    case InputPull::None:
        palSetLineMode(m_line, PAL_MODE_INPUT);
        break;
    case InputPull::Up:
        palSetLineMode(m_line, PAL_MODE_INPUT_PULLUP);
        break;
    case InputPull::Down:
        palSetLineMode(m_line, PAL_MODE_INPUT_PULLDOWN);
        break;
    }
}