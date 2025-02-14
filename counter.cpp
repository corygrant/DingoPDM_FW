#include "counter.h"
#include "edge.h"

void Counter::Update()
{
    if (!pConfig->bEnabled)
    {
        nVal = 0;
        return;
    }

    if (Edge::Check(pConfig->eResetEdge, bLastReset, *pResetInput))
    {
        nVal = 0;
        return;
    }

    if (Edge::Check(pConfig->eIncEdge, bLastInc, *pIncInput))
    {
        nVal++;
        if (nVal > pConfig->nMaxCount)
        {
            nVal = pConfig->bWrapAround ? 0 : pConfig->nMaxCount;
        }
    }

    if (Edge::Check(pConfig->eDecEdge, bLastDec, *pDecInput))
    {
        nVal--;
        if (nVal < pConfig->nMinCount)
        {
            nVal = pConfig->bWrapAround ? pConfig->nMaxCount : pConfig->nMinCount;
        }
    }

    bLastInc = *pIncInput;
    bLastDec = *pDecInput;
    bLastReset = *pResetInput;
}