#pragma once

#include "led.h"
#include "enums.h"

class Error
{
public:
    static void Initialize(Led *status, Led *error);
    static void SetFatalError(FatalErrorType err, MsgSrc src);

private:
    static Led *m_status;
    static Led *m_error;
};
