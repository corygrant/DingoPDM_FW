#include "virtual_input.h"

void VirtualInput::Update()
{
    if (!pConfig->bEnabled)
        return;
    if ((pVar0 == 0) || (pVar1 == 0))
        return;

    bResult0 = *pVar0;
    if (pConfig->bNot0)
        bResult0 = !bResult0;

    bResult1 = *pVar1;
    if (pConfig->bNot1)
        bResult1 = !bResult1;

    switch (pConfig->eCond0)
    {
    case Condition::And:
        bResultSec0 = bResult0 && bResult1;
        break;
    case Condition::Or:
        bResultSec0 = bResult0 || bResult1;
        break;
    case Condition::Nor:
        bResultSec0 = !bResult0 || !bResult1;
        break;
    }

    // Only 2 conditions
    if (pConfig->nVar2 == 0)
    {
        nVal = input.Check(pConfig->eMode, false, bResultSec0);
        return;
    }

    bResult2 = *pVar2;

    if (pConfig->bNot2)
        bResult2 = !bResult2;

    switch (pConfig->eCond1)
    {
    case Condition::And:
        bResultSec1 = bResultSec0 && bResult2;
        return;
    case Condition::Or:
        bResultSec1 = bResultSec0 || bResult2;
        return;
    case Condition::Nor:
        bResultSec1 = !bResultSec0 || !bResult2;
        return;
    }

    nVal = input.Check(pConfig->eMode, false, bResultSec1);
}
