#pragma once

#include "led.h"
#include "enums.h"

class Error
{
public:
    static void Initialize(Led *status, Led *error);
    static void SetFatalError(FatalErrorType err, MsgSrc src);

private:
    static Led *statusLed;
    static Led *errorLed;
};
