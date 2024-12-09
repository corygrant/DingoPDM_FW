#include "error.h"
#include "config.h"
#include "msg.h"

Led* Error::m_status = nullptr;
Led* Error::m_error = nullptr;

void Error::Initialize(Led *status, Led *error)
{
    m_status = status;
    m_error = error;
}

void Error::SetFatalError(FatalErrorType err, MsgSrc src)
{
    static InfoMsg FatalErrorMsg(MsgType::Error, src);
    FatalErrorMsg.Check(true, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    m_status->Solid(false);
    while (true)
    {
        m_error->Code(TIME_I2MS(chVTGetSystemTimeX()), static_cast<uint8_t>(err));
    }
}