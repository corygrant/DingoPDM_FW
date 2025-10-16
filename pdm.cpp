#include "pdm.h"
#include "ch.hpp"
#include "hal.h"
#include "port.h"
#include "mcu_utils.h"
#include "dingopdm_config.h"
#include "config.h"
#include "config_handler.h"
#include "hw_devices.h"
#include "can.h"
#include "can_input.h"
#include "virtual_input.h"
#include "wiper/wiper.h"
#include "starter.h"
#include "flasher.h"
#include "counter.h"
#include "condition.h"
#include "mailbox.h"
#include "msg.h"
#include "error.h"
#include "usb.h"

CanInput canIn[PDM_NUM_CAN_INPUTS];
VirtualInput virtIn[PDM_NUM_VIRT_INPUTS];
Wiper wiper;
Starter starter;
Flasher flasher[PDM_NUM_FLASHERS];
Counter counter[PDM_NUM_COUNTERS];
Condition condition[PDM_NUM_CONDITIONS];

PdmState eState = PdmState::Run;
FatalErrorType eError = FatalErrorType::NoError;

PdmConfig stConfig;
uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

float fBattVolt;
float fTempSensor;
bool bDeviceOverTemp;
bool bDeviceCriticalTemp;
bool bSleepRequest;
bool bBootloaderRequest;

uint8_t nNumOutputsOn;
uint8_t nLastNumOutputsOn;
uint32_t nAllOutputsOffTime;
bool bLastUsbConnected;
uint32_t nUsbDisconnectedTime;

void InitVarMap();
void ApplyAllConfig();
void ApplyConfig(MsgCmd eCmd);
void CyclicUpdate();
void States();
void SendInfoMsgs();
void InitInfoMsgs();
void CheckRequestMsgs(CANRxFrame *frame);
bool GetAnyOvercurrent();
bool GetAnyFault();
bool CheckEnterSleep();
void EnterSleep();
void EnableLineEventWithPull(ioline_t line, InputPull pull);

static InfoMsg StateRunMsg(MsgType::Info, MsgSrc::State_Run);
static InfoMsg StateSleepMsg(MsgType::Info, MsgSrc::State_Sleep);
static InfoMsg StateOvertempMsg(MsgType::Warning, MsgSrc::State_Overtemp);
static InfoMsg StateErrorMsg(MsgType::Error, MsgSrc::State_Error);

static InfoMsg BattOvervoltageMsg(MsgType::Warning, MsgSrc::Voltage);
static InfoMsg BattUndervoltageMsg(MsgType::Warning, MsgSrc::Voltage);

InfoMsg OutputOvercurrentMsg[PDM_NUM_OUTPUTS];
InfoMsg OutputFaultMsg[PDM_NUM_OUTPUTS];

struct PdmThread : chibios_rt::BaseStaticThread<2048>
{
    void main()
    {
        setName("PdmThread");

        while (true)
        {
            CyclicUpdate();
            States();
            palToggleLine(LINE_E1);
            chThdSleepMilliseconds(2);
        }
    }
};
static PdmThread pdmThread;

struct SlowThread : chibios_rt::BaseStaticThread<256>
{
    void main()
    {
        setName("SlowThread");

        while (true)
        {
            //=================================================================
            // Perform tasks that don't need to be done every cycle here
            //=================================================================
            if (chThdShouldTerminateX())
                chThdExit(MSG_OK);

            fBattVolt = GetBattVolt();

            // Temp sensor is I2C, takes a while to read
            // Don't want to slow down main thread
            fTempSensor = tempSensor.GetTemp();
            bDeviceOverTemp = tempSensor.OverTempLimit();
            bDeviceCriticalTemp = tempSensor.CritTempLimit();

            // palToggleLine(LINE_E2);
            chThdSleepMilliseconds(250);
        }
    }
};
static SlowThread slowThread;
static chibios_rt::ThreadReference slowThreadRef;

void InitPdm()
{
    Error::Initialize(&statusLed, &errorLed);

    InitVarMap(); // Set val pointers

    if (!i2cStart(&I2CD1, &i2cConfig) == HAL_RET_SUCCESS)
        Error::SetFatalError(FatalErrorType::ErrI2C, MsgSrc::Init);

    InitConfig(); // Read config from FRAM

    ApplyAllConfig();

    if(!InitAdc() == HAL_RET_SUCCESS)
        Error::SetFatalError(FatalErrorType::ErrADC, MsgSrc::Init);
        
    if(!InitCan(&stConfig.stDevConfig) == HAL_RET_SUCCESS) // Starts CAN threads
        Error::SetFatalError(FatalErrorType::ErrCAN, MsgSrc::Init);

    if (!InitUsb() == HAL_RET_SUCCESS) // Starts USB threads
        Error::SetFatalError(FatalErrorType::ErrUSB, MsgSrc::Init);

    if (!tempSensor.Init(BOARD_TEMP_WARN, BOARD_TEMP_CRIT))
        Error::SetFatalError(FatalErrorType::ErrTempSensor, MsgSrc::Init);

    InitInfoMsgs();

    stConfig.stOutput[0].stPair.eMode = PairOutputMode::Leader;
    stConfig.stOutput[0].stPair.nPairOutNum = 1;
    stConfig.stOutput[0].stPair.bUsePwm = false;

    stConfig.stOutput[1].stPair.eMode = PairOutputMode::Follower;
    stConfig.stOutput[1].stPair.nPairOutNum = 0;
    stConfig.stOutput[1].stPair.bUsePwm = false;

    palClearLine(LINE_CAN_STANDBY); // CAN enabled

    slowThreadRef = slowThread.start(NORMALPRIO);
    pdmThread.start(NORMALPRIO);
}

void States()
{
    if (bDeviceCriticalTemp)
    {
        //Turn off all outputs
        for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
            pf[i].Update(false);
            
        Error::SetFatalError(FatalErrorType::ErrTemp, MsgSrc::State_Overtemp);
    }

    if (eState == PdmState::Run)
    {
        if (bDeviceOverTemp)
            eState = PdmState::OverTemp;

        if (GetAnyOvercurrent() && !GetAnyFault())
        {
            statusLed.Blink();
            errorLed.Solid(false);
        }

        if (GetAnyFault())
        {
            statusLed.Blink();
            errorLed.Solid(true);
        }

        if (!GetAnyOvercurrent() && !GetAnyFault())
        {
            statusLed.Solid(true);
            errorLed.Solid(false);
        }
    }

    if (eState == PdmState::OverTemp)
    {
        statusLed.Blink();
        errorLed.Blink();

        if (!bDeviceOverTemp)
            eState = PdmState::Run;
    }

    if (CheckEnterSleep())
    {
        statusLed.Solid(false);
        errorLed.Solid(false);
        eState = PdmState::Sleep;
    }

    if (eState == PdmState::Sleep)
    {
        bSleepRequest = false;
        palSetLine(LINE_CAN_STANDBY); // CAN disabled
        EnterSleep();
    }

    if (eState == PdmState::Error)
    {
        // Not required?

        //Turn off all outputs
        for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
            pf[i].Update(false);

        Error::SetFatalError(eError, MsgSrc::State_Error);
    }

    SendInfoMsgs();
}

void CyclicUpdate()
{
    CANRxFrame rxMsg;

    while (!RxFramesEmpty())
    {
        msg_t res = FetchRxFrame(&rxMsg);
        if (res == MSG_OK)
        {
            for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
                canIn[i].CheckMsg(rxMsg);

            CheckRequestMsgs(&rxMsg);
            ApplyConfig(ConfigHandler(&rxMsg));
        }
    }

    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        pf[i].Update(starter.nVal[i]);

        pf[i].SetPairState(pf[stConfig.stOutput[i].stPair.nPairOutNum].GetState());

        //Copy leader values if follower
        if( stConfig.stOutput[i].stPair.eMode == PairOutputMode::Follower )
        {
            pf[i].SetFollowerVals(  pf[stConfig.stOutput[i].stPair.nPairOutNum].GetPwmActive(),
                                    pf[stConfig.stOutput[i].stPair.nPairOutNum].GetDutyCycle(),
                                    pf[stConfig.stOutput[i].stPair.nPairOutNum].GetPwmDriver(),
                                    pf[stConfig.stOutput[i].stPair.nPairOutNum].GetPwmChannel());
        }
    }

    for (uint8_t i = 0; i < PDM_NUM_INPUTS; i++)
        in[i].Update();

    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
        canIn[i].CheckTimeout();

    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
        virtIn[i].Update();

    wiper.Update();

    starter.Update();

    for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
        flasher[i].Update(SYS_TIME);

    for (uint8_t i = 0; i < PDM_NUM_COUNTERS; i++)
        counter[i].Update();

    for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
        condition[i].Update();
}

void InitVarMap()
{
    // 0
    // None - set to 0
    pVarMap[0] = const_cast<uint16_t*>(&ALWAYS_FALSE);

    // 1-2
    // Digital inputs
    pVarMap[1] = &in[0].nVal;
    pVarMap[2] = &in[1].nVal;

    // 3-34
    // CAN Inputs
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        pVarMap[i + 3] = &canIn[i].nOutput;
    }

    // 35-66
    // CAN Input val
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        pVarMap[i + 35] = &canIn[i].nVal;
    }

    // 67-84
    // Virtual Inputs
    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
    {
        pVarMap[i + 67] = &virtIn[i].nVal;
    }

    // 83-90
    // Outputs
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        pVarMap[i + 83] = &pf[i].nOutput;
    }

    //dingoPDM-Max
    //Var map 87-90 are not used

    // 91-92
    // Wiper
    pVarMap[91] = &wiper.nSlowOut;
    pVarMap[92] = &wiper.nFastOut;

    // 93-96
    // Flashers
    for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
    {
        pVarMap[i + 93] = &flasher[i].nVal;
    }

    // 97-100
    // Counters
    for (uint8_t i = 0; i < PDM_NUM_COUNTERS; i++)
    {
        pVarMap[i + 97] = &counter[i].nVal;
    }

    // 101 - 132
    // Conditions
    for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
    {
        pVarMap[i + 101] = &condition[i].nVal;
    }

    // 133
    // Always true
    pVarMap[133] = const_cast<uint16_t*>(&ALWAYS_TRUE);
}

void ApplyAllConfig()
{
    ApplyConfig(MsgCmd::Inputs);
    ApplyConfig(MsgCmd::CanInputs);
    ApplyConfig(MsgCmd::VirtualInputs);
    ApplyConfig(MsgCmd::Outputs);
    ApplyConfig(MsgCmd::Wiper);
    ApplyConfig(MsgCmd::StarterDisable);
    ApplyConfig(MsgCmd::Flashers);
    ApplyConfig(MsgCmd::Counters);
    ApplyConfig(MsgCmd::Conditions);
}

void ApplyConfig(MsgCmd eCmd)
{
    if (eCmd == MsgCmd::Can)
    {
        // TODO: Change CAN speed and filters without requiring reset

        SetCanFilterEnabled(stConfig.stDevConfig.bCanFilterEnabled);
    }

    if (eCmd == MsgCmd::Inputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_INPUTS; i++)
            in[i].SetConfig(&stConfig.stInput[i]);
    }

    if ((eCmd == MsgCmd::CanInputs) || (eCmd == MsgCmd::CanInputsId))
    {
        ClearCanFilters(); // Clear all filters before setting new ones

        // Set filter for CAN settings request message, (Base ID - 1)
        // Use filter 0, it is always enabled to allow all messages by hal so it must be used
        SetCanFilterId(0, stConfig.stCanOutput.nBaseId - 1, false);

        for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
        {
            canIn[i].SetConfig(&stConfig.stCanInput[i]);
            if(!stConfig.stCanInput[i].bEnabled)
                continue; // Skip if not enabled
            
            // Set filter for this input
            uint32_t nId = 0;
            if(stConfig.stCanInput[i].nIDE == 1)
                nId = stConfig.stCanInput[i].nEID;
            else
                nId = stConfig.stCanInput[i].nSID;
            SetCanFilterId(i + 1, nId, stConfig.stCanInput[i].nIDE == 1);
        }

        //TODO: Set can filter without requiring reset, need a new message to indicate all IDs set before stopping CAN
    }

    if (eCmd == MsgCmd::VirtualInputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
            virtIn[i].SetConfig(&stConfig.stVirtualInput[i], pVarMap);
    }

    if (eCmd == MsgCmd::Outputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
            pf[i].SetConfig(&stConfig.stOutput[i], pVarMap);
    }

    if ((eCmd == MsgCmd::Wiper) || (eCmd == MsgCmd::WiperSpeed) || (eCmd == MsgCmd::WiperDelays))
    {
        wiper.SetConfig(&stConfig.stWiper, pVarMap);
    }

    if (eCmd == MsgCmd::StarterDisable)
    {
        starter.SetConfig(&stConfig.stStarter, pVarMap);
    }

    if (eCmd == MsgCmd::Flashers)
    {
        for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
            flasher[i].SetConfig(&stConfig.stFlasher[i], pVarMap);
    }

    if (eCmd == MsgCmd::Counters)
    {
        for (uint8_t i = 0; i < PDM_NUM_COUNTERS; i++)
            counter[i].SetConfig(&stConfig.stCounter[i], pVarMap);
    }

    if (eCmd == MsgCmd::Conditions)
    {
        for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
            condition[i].SetConfig(&stConfig.stCondition[i], pVarMap);
    }
}

void CheckRequestMsgs(CANRxFrame *frame)
{
    //Check for settings request message, (Base ID - 1)
    if(frame->SID != (stConfig.stCanOutput.nBaseId - 1))
        return;

    // Check for sleep request
    if ((frame->DLC == 5) && 
        (frame->data8[0] == static_cast<uint8_t>(MsgCmd::Sleep)) &&
        (frame->data8[1] == 'Q') && (frame->data8[2] == 'U') && 
        (frame->data8[3] == 'I') && (frame->data8[4] == 'T'))
    {
        CANTxFrame txMsg;
        txMsg.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        txMsg.IDE = CAN_IDE_STD;
        txMsg.DLC = 2;
        txMsg.data8[0] = static_cast<uint8_t>(MsgCmd::Sleep) + 128;
        txMsg.data8[1] = 1;

        PostTxFrame(&txMsg);

        bSleepRequest = true;
    }

    // Check for burn request
    if ((frame->DLC == 4) && 
        (frame->data8[0] == static_cast<uint8_t>(MsgCmd::BurnSettings)) &&
        (frame->data8[1] == 1) &&
        (frame->data8[2] == 3) && 
        (frame->data8[3] == 8))
    {
        CANTxFrame txMsg;
        txMsg.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        txMsg.IDE = CAN_IDE_STD;
        txMsg.DLC = 2;
        txMsg.data8[0] = static_cast<uint8_t>(MsgCmd::BurnSettings) + 128;
        txMsg.data8[1] = WriteConfig();

        PostTxFrame(&txMsg);
    }

    // Check for bootloader request
    if ((frame->DLC == 6) &&
        (frame->data8[0] == static_cast<uint8_t>(MsgCmd::Bootloader)) && 
        (frame->data8[1] == 'B') && (frame->data8[2] == 'O') && 
        (frame->data8[3] == 'O') && (frame->data8[4] == 'T') && (frame->data8[5] == 'L'))
    {
        RequestBootloader();
    }

    // Check for version request
    if ((frame->DLC == 1) &&
        (frame->data8[0] == static_cast<uint8_t>(MsgCmd::Version)))
    {
        CANTxFrame txMsg;
        txMsg.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        txMsg.IDE = CAN_IDE_STD;
        txMsg.DLC = 5;
        txMsg.data8[0] = static_cast<uint8_t>(MsgCmd::Version) + 128;
        txMsg.data8[1] = MAJOR_VERSION;
        txMsg.data8[2] = MINOR_VERSION;
        txMsg.data8[3] = BUILD >> 8;
        txMsg.data8[4] = BUILD & 0xFF;

        PostTxFrame(&txMsg);
    }
}

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
        OutputOvercurrentMsg[i].Check(pf[i].GetState() == ProfetState::Overcurrent, stConfig.stCanOutput.nBaseId, i, pf[i].GetCurrent(), 0);
        OutputFaultMsg[i].Check(pf[i].GetState() == ProfetState::Fault, stConfig.stCanOutput.nBaseId, i, pf[i].GetCurrent(), 0);
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

uint8_t GetOutputDC(uint8_t nOutput)
{
    if (nOutput >= PDM_NUM_OUTPUTS)
        return 0;

    return pf[nOutput].GetDutyCycle();
}

bool GetAnyCanInEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        if (canIn[i].GetEnable())
            return true;
    }
    return false;
}

bool GetCanInEnable(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return false;

    return canIn[nInput].GetEnable();
}

bool GetCanInOutput(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return false;

    return canIn[nInput].nOutput;
}

uint16_t GetCanInVal(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return false;

    return canIn[nInput].nVal;
}

bool GetAnyVirtInEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
    {
        if (virtIn[i].GetEnable())
            return true;
    }
    return false;
}

bool GetVirtInVal(uint8_t nInput)
{
    if (nInput >= PDM_NUM_VIRT_INPUTS)
        return false;

    return virtIn[nInput].nVal;
}

bool GetWiperEnable()
{
    return wiper.GetEnable();
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

bool GetAnyFlasherEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
    {
        if (flasher[i].GetEnable())
            return true;
    }
    return false;
}

bool GetFlasherVal(uint8_t nFlasher)
{
    if (nFlasher >= PDM_NUM_FLASHERS)
        return false;

    return flasher[nFlasher].nVal;
}

bool GetAnyCounterEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_COUNTERS; i++)
    {
        if (counter[i].GetEnable())
            return true;
    }
    return false;
}

uint16_t GetCounterVal(uint8_t nCounter)
{
    if (nCounter >= PDM_NUM_COUNTERS)
        return 0;

    return counter[nCounter].nVal;
}

bool GetAnyConditionEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
    {
        if (condition[i].GetEnable())
            return true;
    }
    return false;
}

bool GetConditionVal(uint8_t nCondition)
{
    if (nCondition >= PDM_NUM_CONDITIONS)
        return false;

    return condition[nCondition].nVal;
}

bool CheckEnterSleep()
{
    bool bEnterSleep = false;

    // Count number of outputs on
    nNumOutputsOn = 0;
    for (int i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        if (pf[i].GetState() != ProfetState::Off)
            nNumOutputsOn++;
    }

    // All outputs just turned off, save time - wait SLEEP_TIMEOUT before sleep
    if ((nNumOutputsOn == 0) && (nLastNumOutputsOn > 0))
    {
        nAllOutputsOffTime = SYS_TIME;
    }
    nLastNumOutputsOn = nNumOutputsOn;

    //USB disconnected, save time - wait SLEEP_TIMEOUT before sleep
    if (!GetUsbConnected() && bLastUsbConnected)
        nUsbDisconnectedTime = SYS_TIME;

    bLastUsbConnected = GetUsbConnected();

    // Had issue with SYS_TIME being < GetLastCanRxTime when msgs come in quickly
    // Check that SYS_TIME is greater than or equal to GetLastCanRxTime
    uint32_t sys = SYS_TIME;
    uint32_t canLast = GetLastCanRxTime();
    uint32_t nCanRxIdleTime;
    if (sys >= canLast)
        nCanRxIdleTime = sys - canLast;
    else
        nCanRxIdleTime = 0;

    // No outputs on, no CAN msgs received and no USB connected
    // Go to sleep after timeout
    bEnterSleep = stConfig.stDevConfig.bSleepEnabled &&
                  (nNumOutputsOn == 0) &&
                  (nLastNumOutputsOn == 0) &&
                  !GetUsbConnected() &&
                  ((SYS_TIME - nUsbDisconnectedTime) > SLEEP_TIMEOUT) &&
                  ((SYS_TIME - nAllOutputsOffTime) > SLEEP_TIMEOUT) &&
                  (nCanRxIdleTime > SLEEP_TIMEOUT);

    if (nCanRxIdleTime > SLEEP_TIMEOUT)
    {
        nCanRxIdleTime = SYS_TIME - GetLastCanRxTime();
    }

    return bEnterSleep || bSleepRequest;
}

void EnableLineEventWithPull(ioline_t line, InputPull pull) {
    uint32_t eventMode = PAL_EVENT_MODE_BOTH_EDGES;
    
    switch(pull) {
        case InputPull::Up:
            eventMode |= PAL_STM32_PUPDR_PULLUP;
            break;
        case InputPull::Down:
            eventMode |= PAL_STM32_PUPDR_PULLDOWN;
            break;
        default:
            eventMode |= PAL_STM32_PUPDR_FLOATING;
            break;
    }
    
    palEnableLineEvent(line, eventMode);
}

void EnterSleep()
{
    // Set wakeup sources

    // Digital inputs change detection, with configured pullup or pulldown
    EnableLineEventWithPull(LINE_DI1, stConfig.stInput[0].ePull);
    EnableLineEventWithPull(LINE_DI2, stConfig.stInput[1].ePull);

    // CAN receive detection
    palSetLineMode(LINE_CAN_RX, PAL_MODE_INPUT);
    palEnableLineEvent(LINE_CAN_RX, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_FLOATING);

    // USB detection
    palSetLineMode(LINE_USB_DP, PAL_MODE_INPUT);
    palEnableLineEvent(LINE_USB_DP, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_FLOATING);
    palSetLineMode(LINE_USB_DM, PAL_MODE_INPUT);
    palEnableLineEvent(LINE_USB_DM, PAL_EVENT_MODE_BOTH_EDGES | PAL_STM32_PUPDR_FLOATING);

    EnterStopMode();
}

