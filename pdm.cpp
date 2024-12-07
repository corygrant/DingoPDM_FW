#include "pdm.h"
#include "ch.hpp"
#include "hal.h"
#include "dingopdm_config.h"
#include "config.h"
#include "config_handler.h"
#include "profet.h"
#include "digital.h"
#include "analog.h"
#include "can.h"
#include "can_input.h"
#include "virtual_input.h"
#include "wiper.h"
#include "starter.h"
#include "flasher.h"
#include "led.h"
#include "mailbox.h"
#include "msg.h"
#include "hardware/mcp9808.h"

uint16_t nAlwaysTrue = 1;

PdmConfig stConfig;

Profet pf[PDM_NUM_OUTPUTS] = {
    Profet(1, ProfetModel::BTS7002_1EPP, LINE_PF1_IN, LINE_PF1_DEN, LINE_UNUSED, AnalogChannel::IS1),
    Profet(2, ProfetModel::BTS7002_1EPP, LINE_PF2_IN, LINE_PF2_DEN, LINE_UNUSED, AnalogChannel::IS2),
    Profet(3, ProfetModel::BTS7008_2EPA_CH1, LINE_PF3_IN, LINE_PF3_4_DEN, LINE_PF3_4_DSEL, AnalogChannel::IS3_4),
    Profet(4, ProfetModel::BTS7008_2EPA_CH2, LINE_PF4_IN, LINE_PF3_4_DEN, LINE_PF3_4_DSEL, AnalogChannel::IS3_4),
    Profet(5, ProfetModel::BTS7008_2EPA_CH1, LINE_PF5_IN, LINE_PF5_6_DEN, LINE_PF5_6_DSEL, AnalogChannel::IS5_6),
    Profet(6, ProfetModel::BTS7008_2EPA_CH2, LINE_PF6_IN, LINE_PF5_6_DEN, LINE_PF5_6_DSEL, AnalogChannel::IS5_6),
    Profet(7, ProfetModel::BTS7008_2EPA_CH1, LINE_PF7_IN, LINE_PF7_8_DEN, LINE_PF7_8_DSEL, AnalogChannel::IS7_8),
    Profet(8, ProfetModel::BTS7008_2EPA_CH2, LINE_PF8_IN, LINE_PF7_8_DEN, LINE_PF7_8_DSEL, AnalogChannel::IS7_8)};

Digital in[PDM_NUM_INPUTS] = {
    Digital(DigitalChannel::In1),
    Digital(DigitalChannel::In2)};

CanInput canIn[PDM_NUM_CAN_INPUTS];
VirtualInput virtIn[PDM_NUM_VIRT_INPUTS];
Wiper wiper;
Starter starter;
Flasher flasher[PDM_NUM_FLASHERS];

Led statusLed = Led(LedType::Status);
Led errorLed = Led(LedType::Error);

PdmState eState = PdmState::PowerOn;

uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

float fBattVolt;
float fVref;
float fTemp;
float fTempSensor;
int16_t signedTemp;
uint16_t rawTemp;
bool tempSensorOk;
bool bDeviceOverTemp;
bool bDeviceCriticalTemp;

uint8_t testRead[100];
uint8_t testWrite[3];
bool framOk;

MCP9808 tempSensor(I2CD1, MCP9808_I2CADDR_DEFAULT);

void InitVarMap();
void ApplyConfig();
void SetConfig(MsgCmdRx eCmd);

void CyclicUpdate();
void StateMachine();

void CheckStateMsgs();
void CheckRequestMsgs(CANRxFrame *frame);

bool GetAnyOvercurrent();
bool GetAnyFault();

struct PdmThread : chibios_rt::BaseStaticThread<256>
{
    void main()
    {
        while (true)
        {
            CyclicUpdate();
            StateMachine();
            palToggleLine(LINE_E1);
            chThdSleepMilliseconds(1);
        }
    }
};
static PdmThread pdmThread;

struct SlowThread : chibios_rt::BaseStaticThread<256>
{
    void main()
    {
        while (true)
        {
            //=================================================================
            // Perform tasks that don't need to be done every cycle here
            //=================================================================

            fBattVolt = GetBattVolt();
            fVref = GetVDDA();
            fTemp = GetTemperature();

            // Temp sensor is I2C, takes a while to read
            // Don't want to slow down main thread
            fTempSensor = tempSensor.GetTemp();
            bDeviceOverTemp = tempSensor.OverTempLimit();
            bDeviceCriticalTemp = tempSensor.CritTempLimit();

            palToggleLine(LINE_E2);
            chThdSleepMilliseconds(250);
        }
    }
};
static SlowThread slowThread;

void InitPdm()
{
    InitVarMap(); // Set val pointers

    i2cStart(&I2CD1, &i2cConfig);

    InitConfig(); // Read config from FRAM
    ApplyConfig();

    InitAdc(); // Start ADC thread
    InitCan(); // Start CAN thread

    tempSensorOk = tempSensor.Init();

    slowThread.start(NORMALPRIO);
    pdmThread.start(NORMALPRIO);
}

void StateMachine()
{
    CheckStateMsgs();

    switch (eState)
    {

    case PdmState::PowerOn:
        // Nothing to do...yet
        eState = PdmState::Starting;
        break;

    case PdmState::Starting:
        // Nothing to do...yet
        eState = PdmState::Run;
        break;

    case PdmState::Run:

        if (GetAnyOvercurrent() && !GetAnyFault())
        {
            statusLed.Blink(chVTGetSystemTimeX());
            errorLed.Solid(false);
        }

        if (GetAnyFault())
        {
            statusLed.Blink(chVTGetSystemTimeX());
            errorLed.Solid(true);
        }

        if (!GetAnyOvercurrent() && !GetAnyFault())
        {
            statusLed.Solid(true);
            errorLed.Solid(false);
        }

        /*
        if any overcurrent, send overcurrent message

        if any fault, send fault message

        if board overtemp, send overtemp message
            set to overtemp state

        if board overtemp, send overtemp message
            set to error state

        if battery voltage too low or too high
            send error message

        check for sleep request
        */

        break;

    case PdmState::Sleep:

        /*
        perform necessary tasks to put the PDM to sleep
        */

        eState = PdmState::Wake;
        break;

    case PdmState::Wake:
        /*
        perform necessary tasks to wake the PDM
        */
        eState = PdmState::Run;
        break;

    case PdmState::OverTemp:
        statusLed.Blink(chVTGetSystemTimeX());
        errorLed.Blink(chVTGetSystemTimeX());

        if (bDeviceCriticalTemp)
            eState = PdmState::Error;

        if (!bDeviceOverTemp)
            eState = PdmState::Run;

        break;

    case PdmState::Error:
        statusLed.Solid(false);
        /*
        send fatal error message
        set led flashing code
        trap execution, requires power cycle
        */
        break;
    }
}

void CyclicUpdate()
{
    CANRxFrame rxMsg;

    msg_t res = FetchRxFrame(&rxMsg);
    if (res == MSG_OK)
    {
        for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
            canIn[i].CheckMsg(rxMsg);

        CheckRequestMsgs(&rxMsg);
        SetConfig(ConfigHandler(&rxMsg));
    }

    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
        pf[i].Update((eState == PdmState::Run) && starter.nVal[i]);

    for (uint8_t i = 0; i < PDM_NUM_INPUTS; i++)
        in[i].Update();

    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
        virtIn[i].Update();

    wiper.Update(chVTGetSystemTimeX());

    starter.Update();

    for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
        flasher[i].Update(chVTGetSystemTimeX());
}

void InitVarMap()
{
    // 1-2
    // Digital inputs
    pVarMap[1] = &in[0].nVal;
    pVarMap[2] = &in[1].nVal;

    // 3-34
    // CAN Inputs
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        pVarMap[i + 3] = &canIn[i].nVal;
    }

    // 35-50
    // Virtual Inputs
    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
    {
        pVarMap[i + 35] = &virtIn[i].nVal;
    }

    // 51-66
    // Outputs
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        pVarMap[i + 51] = &pf[i].nOutput;
    }

    // 59-60
    // Wiper
    pVarMap[59] = &wiper.nSlowOut;
    pVarMap[60] = &wiper.nFastOut;

    // 61-64
    // Flashers
    for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
    {
        pVarMap[i + 61] = &flasher[i].nVal;
    }

    // 65
    // Always true
    pVarMap[65] = &nAlwaysTrue;
}

void ApplyConfig()
{
    SetConfig(MsgCmdRx::Inputs);
    SetConfig(MsgCmdRx::CanInputs);
    SetConfig(MsgCmdRx::VirtualInputs);
    SetConfig(MsgCmdRx::Outputs);
    SetConfig(MsgCmdRx::Wiper);
    SetConfig(MsgCmdRx::Starter);
    SetConfig(MsgCmdRx::Flashers);
}

void SetConfig(MsgCmdRx eCmd)
{
    if (eCmd == MsgCmdRx::Can)
    {
        // TODO: Change CAN speed
    }

    if (eCmd == MsgCmdRx::Inputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_INPUTS; i++)
            in[i].SetConfig(&stConfig.stInput[i]);
    }

    if (eCmd == MsgCmdRx::CanInputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
            canIn[i].SetConfig(&stConfig.stCanInput[i]);
    }

    if (eCmd == MsgCmdRx::VirtualInputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
            virtIn[i].SetConfig(&stConfig.stVirtualInput[i], pVarMap);
    }

    if (eCmd == MsgCmdRx::Outputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
            pf[i].SetConfig(&stConfig.stOutput[i], pVarMap);
    }

    if ((eCmd == MsgCmdRx::Wiper) || (eCmd == MsgCmdRx::WiperSpeed) || (eCmd == MsgCmdRx::WiperDelays))
    {
        wiper.SetConfig(&stConfig.stWiper, pVarMap);
    }

    if (eCmd == MsgCmdRx::Starter)
    {
        starter.SetConfig(&stConfig.stStarter, pVarMap);
    }

    if (eCmd == MsgCmdRx::Flashers)
    {
        for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
            flasher[i].SetConfig(&stConfig.stFlasher[i], pVarMap);
    }
}

void CheckRequestMsgs(CANRxFrame *frame)
{
        // Check for sleep request
        if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Sleep))
        {
            CANTxFrame txMsg;
            txMsg.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            txMsg.IDE = CAN_IDE_STD;
            txMsg.DLC = 2;
            txMsg.data8[0] = static_cast<uint8_t>(MsgCmdTx::Sleep);
            txMsg.data8[1] = 1;

            PostTxFrame(&txMsg);

            // TODO: Set sleep state
        }

        // Check for burn request
        if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::BurnSettings))
        {
            CANTxFrame txMsg;
            txMsg.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            txMsg.IDE = CAN_IDE_STD;
            txMsg.DLC = 2;
            txMsg.data8[0] = static_cast<uint8_t>(MsgCmdTx::BurnSettings);
            txMsg.data8[1] = WriteConfig();

            PostTxFrame(&txMsg);
        }

        // Check for bootloader request
        if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Bootloader))
        {
            // TODO: Enter bootloader
        }

        // Check for version request
        if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::GetVersion))
        {
            CANTxFrame txMsg;
            txMsg.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            txMsg.IDE = CAN_IDE_STD;
            txMsg.DLC = 5;
            txMsg.data8[0] = static_cast<uint8_t>(MsgCmdTx::GetVersion);
            txMsg.data8[1] = MAJOR_VERSION;
            txMsg.data8[2] = MINOR_VERSION;
            txMsg.data8[3] = BUILD >> 8;
            txMsg.data8[4] = BUILD & 0xFF;

            PostTxFrame(&txMsg);
        }
}

void CheckStateMsgs()
{
    static InfoMsg StatePowerOnMsg(MsgType::Info, MsgSrc::State_PowerOn);
    static InfoMsg StateStartingMsg(MsgType::Info, MsgSrc::State_Starting);
    static InfoMsg StateRunMsg(MsgType::Info, MsgSrc::State_Run);
    static InfoMsg StateSleepMsg(MsgType::Info, MsgSrc::State_Sleep);
    static InfoMsg StateWakeMsg(MsgType::Info, MsgSrc::State_Wake);
    static InfoMsg StateOvertempMsg(MsgType::Error, MsgSrc::State_Overtemp);
    static InfoMsg StateErrorMsg(MsgType::Error, MsgSrc::State_Error);

    StatePowerOnMsg.Check(eState == PdmState::PowerOn, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateStartingMsg.Check(eState == PdmState::Starting, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateRunMsg.Check(eState == PdmState::Run, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateSleepMsg.Check(eState == PdmState::Sleep, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateWakeMsg.Check(eState == PdmState::Wake, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateOvertempMsg.Check(eState == PdmState::OverTemp, stConfig.stCanOutput.nBaseId, 0, 0, 0);
    StateErrorMsg.Check(eState == PdmState::Error, stConfig.stCanOutput.nBaseId, 0, 0, 0);
}

PdmState GetPdmState()
{
    return eState;
}

float GetBoardTemp()
{
    return fTempSensor;
}

float GetTotalCurrent()
{
    float fTotalCurrent = 0.0;

    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        fTotalCurrent += pf[i].GetCurrent();
    }

    return fTotalCurrent;
}

bool GetAnyOvercurrent()
{
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        if (pf[i].GetState() == ProfetState::Overcurrent)
        {
            // TODO: Send overcurrent message
            return true;
        }
    }

    return false;
}

bool GetAnyFault()
{
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        if (pf[i].GetState() == ProfetState::Fault)
        {
            // TODO: Send fault message
            return true;
        }
    }

    return false;
}

bool GetInputVal(uint8_t nInput)
{
    if (nInput >= PDM_NUM_INPUTS)
        return false;

    return in[nInput].nVal;
}

uint16_t GetOutputCurrent(uint8_t nOutput)
{
    if (nOutput >= PDM_NUM_OUTPUTS)
        return 0;

    return pf[nOutput].GetCurrent();
}

ProfetState GetOutputState(uint8_t nOutput)
{
    if (nOutput >= PDM_NUM_OUTPUTS)
        return ProfetState::Off;

    return pf[nOutput].GetState();
}

uint8_t GetOutputOcCount(uint8_t nOutput)
{
    if (nOutput >= PDM_NUM_OUTPUTS)
        return 0;

    return pf[nOutput].GetOcCount();
}

bool GetCanInVal(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return false;

    return canIn[nInput].nVal;
}

bool GetVirtInVal(uint8_t nInput)
{
    if (nInput >= PDM_NUM_VIRT_INPUTS)
        return false;

    return virtIn[nInput].nVal;
}

bool GetWiperFastOut()
{
    return wiper.nFastOut;
}

bool GetWiperSlowOut()
{
    return wiper.nSlowOut;
}

WiperState GetWiperState()
{
    return wiper.GetState();
}

WiperSpeed GetWiperSpeed()
{
    return wiper.GetSpeed();
}

bool GetFlasherVal(uint8_t nFlasher)
{
    if (nFlasher >= PDM_NUM_FLASHERS)
        return false;

    return flasher[nFlasher].nVal;
}