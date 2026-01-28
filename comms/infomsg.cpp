#include "infomsg.h"
#include "pdm.h"
#include "config.h"
#include "dingopdm_config.h"
#include "status.h"

// Static message objects that were in pdm.cpp
static InfoMsg StateRunMsg(MsgType::Info, MsgSrc::State_Run);
static InfoMsg StateSleepMsg(MsgType::Info, MsgSrc::State_Sleep);
static InfoMsg StateOvertempMsg(MsgType::Error, MsgSrc::State_Overtemp);
static InfoMsg StateErrorMsg(MsgType::Error, MsgSrc::State_Error);

static InfoMsg BattOvervoltageMsg(MsgType::Warning, MsgSrc::Voltage);
static InfoMsg BattUndervoltageMsg(MsgType::Warning, MsgSrc::Voltage);

static InfoMsg OutputOvercurrentMsg[PDM_NUM_OUTPUTS];
static InfoMsg OutputFaultMsg[PDM_NUM_OUTPUTS];

// External variables from pdm.cpp that we need access to
extern PdmConfig stConfig;
extern PdmState eState;
extern float fBattVolt;
extern Profet pf[PDM_NUM_OUTPUTS];

void SendInfoMsgs()
{
    StateRunMsg.Check(eState == PdmState::Run, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateSleepMsg.Check(eState == PdmState::Sleep, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateOvertempMsg.Check(eState == PdmState::OverTemp, stConfig.stCanOutput.nBaseId, GetBoardTemp() * 10, 0, 0);
    StateErrorMsg.Check(eState == PdmState::Error, stConfig.stCanOutput.nBaseId, GetBoardTemp() * 10, GetTotalCurrent() * 10, 0);

    BattOvervoltageMsg.Check(fBattVolt > BATT_HIGH_VOLT, stConfig.stCanOutput.nBaseId, fBattVolt * 10, 0, 0);
    BattUndervoltageMsg.Check(fBattVolt < BATT_LOW_VOLT, stConfig.stCanOutput.nBaseId, fBattVolt * 10, 0, 0);

    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        OutputOvercurrentMsg[i].Check(GetOutputState(i) == ProfetState::Overcurrent, stConfig.stCanOutput.nBaseId, i, GetOutputCurrent(i), 0);
        OutputFaultMsg[i].Check(GetOutputState(i) == ProfetState::Fault, stConfig.stCanOutput.nBaseId, i, GetOutputCurrent(i), 0);
    }
}

void InitInfoMsgs()
{
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        OutputOvercurrentMsg[i] = InfoMsg(MsgType::Warning, MsgSrc::Overcurrent);
        OutputFaultMsg[i] = InfoMsg(MsgType::Error, MsgSrc::Overcurrent);
    }
}

void InfoMsg::Check(bool bTrigger, uint16_t nId, uint16_t nData0, uint16_t nData1, uint16_t nData2)
{
    if (!bTrigger)
    {
        bSent = false;
        return;
    }

    if (bSent)
        return;

    CANTxFrame tx;
    tx.DLC = 8;

    tx.data8[0] = static_cast<uint8_t>(m_type);
    tx.data8[1] = static_cast<uint8_t>(m_src);
    tx.data16[1] = nData0;
    tx.data16[2] = nData1;
    tx.data16[3] = nData2;

    tx.SID = nId + TX_MSG_ID_OFFSET;
    tx.IDE = CAN_IDE_STD;
    PostTxFrame(&tx);

    bSent = true;
}