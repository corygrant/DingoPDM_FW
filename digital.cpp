#include "digital.h"
#include "input.h"

void Digital::Update()
{
    bool bIn;

    bIn = palReadLine(m_line);

    // Debounce input
    if (bIn != bLast)
    {
        nLastTrigTime = chVTGetSystemTimeX();
        bCheck = true;
    }

    bLast = bIn;

    if ((bCheck && ((chVTGetSystemTimeX() - nLastTrigTime) > pConfig->nDebounceTime)) || (!bInit))
    {
        bCheck = false;
        nVal = input.Check(pConfig->eMode, pConfig->bInvert, bIn);
    }

    bInit = true;
}

void Digital::SetPull(InputPull pull)
{
    // TODO: Implement pull-up and pull-down

    switch (pull)
    {
    case InputPull::None:
        break;
    case InputPull::Up:
        break;
    case InputPull::Down:
        break;
    }
}