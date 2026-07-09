#pragma once

#include "port.h"
#include "enums.h"

class Input
{
public:
    Input()
    {
    }

    bool Check(InputMode eMode, bool bInvert, bool bVal);

private:
    bool bOut;
    bool bLast;
};