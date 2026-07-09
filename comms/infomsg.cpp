#include "infomsg.h"
#include "device.h"
#include "config.h"
#include "device_config.h"
#include "status.h"

static InfoMsg StateRunMsg(MsgType::Info, MsgSrc::State_Run);
#if CAN_SLEEP
static InfoMsg StateSleepMsg(MsgType::Info, MsgSrc::State_Sleep);
#endif
#if HAS_EXT_TEMP_SENSOR
static InfoMsg StateOvertempMsg(MsgType::Error, MsgSrc::State_Overtemp);
#endif
static InfoMsg StateErrorMsg(MsgType::Error, MsgSrc::State_Error);

#if HAS_BATT_VOLT_SENSE
static InfoMsg BattOvervoltageMsg(MsgType::Warning, MsgSrc::Voltage);
static InfoMsg BattUndervoltageMsg(MsgType::Warning, MsgSrc::Voltage);
#endif

#if NUM_OUTPUTS > 0
static InfoMsg OutputOvercurrentMsg[NUM_OUTPUTS];
static InfoMsg OutputFaultMsg[NUM_OUTPUTS];
#endif

extern DeviceConfig stConfig;
extern DeviceState eState;

#if HAS_BATT_VOLT_SENSE
extern float fBattVolt;
#endif
#if NUM_OUTPUTS > 0
extern Profet pf[NUM_OUTPUTS];
#endif

void CheckInfoMsgs()
{
    StateRunMsg.Check(eState == DeviceState::Run, stConfig.stDevice.nBaseId, 0, 0, 0);

    #if CAN_SLEEP
    StateSleepMsg.Check(eState == DeviceState::Sleep, stConfig.stDevice.nBaseId, 0, 0, 0);
    #endif
    #if HAS_EXT_TEMP_SENSOR
    StateOvertempMsg.Check(eState == DeviceState::OverTemp, stConfig.stDevice.nBaseId, GetBoardTemp() * 10, 0, 0);
    #endif
    #if (NUM_OUTPUTS > 0 && HAS_EXT_TEMP_SENSOR)
    StateErrorMsg.Check(eState == DeviceState::Error, stConfig.stDevice.nBaseId, GetBoardTemp() * 10, GetTotalCurrent() * 10, 0);
    #else
    StateErrorMsg.Check(eState == DeviceState::Error, stConfig.stDevice.nBaseId, 0, 0, 0);
    #endif

    #if HAS_BATT_VOLT_SENSE
    BattOvervoltageMsg.Check(fBattVolt > BATT_HIGH_VOLT, stConfig.stDevice.nBaseId, fBattVolt * 10, 0, 0);
    BattUndervoltageMsg.Check(fBattVolt < BATT_LOW_VOLT, stConfig.stDevice.nBaseId, fBattVolt * 10, 0, 0);
    #endif

    #if NUM_OUTPUTS > 0
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
    {
        OutputOvercurrentMsg[i].Check(GetOutputState(i) == ProfetState::Overcurrent, stConfig.stDevice.nBaseId, i, GetOutputCurrent(i), 0);
        OutputFaultMsg[i].Check(GetOutputState(i) == ProfetState::Fault, stConfig.stDevice.nBaseId, i, GetOutputCurrent(i), 0);
    }
    #endif
}

void SendInfoMsg(MsgType type, MsgSrc src, uint16_t nId, uint16_t nData0, uint16_t nData1, uint16_t nData2)
{
    CANTxFrame tx;
    tx.DLC = 8;

    tx.data8[0] = static_cast<uint8_t>(type);
    tx.data8[1] = static_cast<uint8_t>(src);
    tx.data16[1] = nData0;
    tx.data16[2] = nData1;
    tx.data16[3] = nData2;

    tx.SID = nId + CONFIG_TX_OFFSET;
    tx.IDE = CAN_IDE_STD;
    PostTxFrame(&tx);
}

void InitInfoMsgs()
{
    #if NUM_OUTPUTS > 0
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
    {
        OutputOvercurrentMsg[i] = InfoMsg(MsgType::Warning, MsgSrc::Overcurrent);
        OutputFaultMsg[i] = InfoMsg(MsgType::Error, MsgSrc::Overcurrent);
    }
    #endif
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

    SendInfoMsg(m_type, m_src, nId, nData0, nData1, nData2);
    bSent = true;
}