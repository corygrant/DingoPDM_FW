#include "error.h"
#include "config.h"
#include "infomsg.h"

Led* Error::statusLed = nullptr;
Led* Error::errorLed = nullptr;

void Error::Initialize(Led *status, Led *error)
{
    statusLed = status;
    errorLed = error;
}

void Error::SetFatalError(FatalErrorType err, MsgSrc src)
{
    static InfoMsg FatalErrorMsg(MsgType::Error, src);
    FatalErrorMsg.Check(true, stConfig.stCanOutput.nBaseId, static_cast<uint8_t>(err), 0, 0);
    statusLed->Solid(false);
    while (true)
    {
        errorLed->Code(static_cast<uint8_t>(err));
    }
}