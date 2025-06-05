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
#include "keypad/keypad.h"

CanInput canIn[PDM_NUM_CAN_INPUTS];
VirtualInput virtIn[PDM_NUM_VIRT_INPUTS];
Wiper wiper;
Starter starter;
Flasher flasher[PDM_NUM_FLASHERS];
Counter counter[PDM_NUM_COUNTERS];
Condition condition[PDM_NUM_CONDITIONS];

Keypad keypad[PDM_NUM_KEYPADS];

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
static InfoMsg StateOvertempMsg(MsgType::Error, MsgSrc::State_Overtemp);
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

    for(uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
    {
        stConfig.stKeypad[0].stButton[i].bEnabled = true;
        stConfig.stKeypad[0].stButton[i].bHasDial = false;
        stConfig.stKeypad[0].stButton[i].eMode = InputMode::Momentary;
    }

    for(uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
        keypad[i].Init();

    InitInfoMsgs();

    palClearLine(LINE_CAN_STANDBY); // CAN enabled

    slowThreadRef = slowThread.start(NORMALPRIO);
    pdmThread.start(NORMALPRIO);
}

void States()
{

    if (eState == PdmState::Run)
    {
        if (bDeviceOverTemp)
            eState = PdmState::OverTemp;
    }

    if (eState == PdmState::OverTemp)
    {
        statusLed.Blink();
        errorLed.Blink();

        if (!bDeviceOverTemp)
            eState = PdmState::Run;
    }

    if ((eState == PdmState::Run) || (eState == PdmState::OverTemp))
    {
        if (bDeviceCriticalTemp)
            Error::SetFatalError(FatalErrorType::ErrTemp, MsgSrc::State_Overtemp);

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

        if (CheckEnterSleep())
        {
            statusLed.Solid(false);
            errorLed.Solid(false);
            eState = PdmState::Sleep;
        }
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

            for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
                keypad[i].CheckMsg(rxMsg);

            CheckRequestMsgs(&rxMsg);
            ApplyConfig(ConfigHandler(&rxMsg));
        }
    }

    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
        pf[i].Update((eState == PdmState::Run) && starter.nVal[i]);

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

    for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
        keypad[i].CheckTimeout();
}

void InitVarMap()
{
    //Numbers in comments below are only valid for dingoPDM/dingoPDM-Max
    //Other combinations of inputs/outputs will have different numbers

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

    // 83-106
    // Outputs
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i += 3)
    {
        pVarMap[i + 83] = &pf[i].nOutput;
        pVarMap[i + 83 + 1] = &pf[i].nOvercurrent;
        pVarMap[i + 83 + 2] = &pf[i].nFault;
    }

    //dingoPDM-Max
    //Var map 95-106 are not used

    // 107-112
    // Wiper
    pVarMap[107] = &wiper.nSlowOut;
    pVarMap[108] = &wiper.nFastOut;
    pVarMap[109] = &wiper.nParkOut;
    pVarMap[110] = &wiper.nInterOut;
    pVarMap[111] = &wiper.nWashOut;
    pVarMap[112] = &wiper.nSwipeOut;

    // 113-116
    // Flashers
    for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
    {
        pVarMap[i + 113] = &flasher[i].nVal;
    }

    // 117-120
    // Counters
    for (uint8_t i = 0; i < PDM_NUM_COUNTERS; i++)
    {
        pVarMap[i + 117] = &counter[i].nVal;
    }

    // 121-152
    // Conditions
    for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
    {
        pVarMap[i + 121] = &condition[i].nVal;
    }

    // 153 - 192
    // Keypad Buttons
    for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
    {
        for (uint8_t j = 0; j < KEYPAD_MAX_BUTTONS; j++)
        {
            pVarMap[i * KEYPAD_MAX_BUTTONS + j + 153] = &keypad[i].nVal[j];
        }
    }

    // 193 - 200
    // Keypad Dials
    for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
    {
        for (uint8_t j = 0; j < KEYPAD_MAX_DIALS; j++)
        {
            pVarMap[i * KEYPAD_MAX_DIALS + j + 193] = &keypad[i].nDialVal[j];
        }
    }

    // 201
    // Always true
    pVarMap[201] = const_cast<uint16_t*>(&ALWAYS_TRUE);


    //Note: Var map max size 255
    // 1 byte per var used in CAN messages
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
    ApplyConfig(MsgCmd::Keypad);
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

    if ((eCmd == MsgCmd::Keypad) || (eCmd == MsgCmd::KeypadLed) || (eCmd == MsgCmd::KeypadButton) ||
        (eCmd == MsgCmd::KeypadButtonLed) || (eCmd == MsgCmd::KeypadDial))
    {
        for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
            keypad[i].SetConfig(&stConfig.stKeypad[i], pVarMap);
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

bool GetAnyPwmEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        if (stConfig.stOutput[i].stPwm.bEnabled)
            return true;
    }
    return false;
}

bool GetAnyCanInEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        if (stConfig.stCanInput[i].bEnabled)
            return true;
    }
    return false;
}

bool GetCanInEnable(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return false;

    return stConfig.stCanInput[nInput].bEnabled;
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

uint32_t GetCanInOutputs()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++) {
        result |= ((canIn[i].nVal & 0x01) << i);
    }
    
    return result;
}

bool GetAnyVirtInEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
    {
        if (stConfig.stVirtualInput[i].bEnabled)
            return true;
    }
    return false;
}

uint32_t GetVirtIns()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++) {
        result |= ((virtIn[i].nVal & 0x01) << i);
    }
    
    return result;
}

bool GetWiperEnable()
{
    return stConfig.stWiper.bEnabled;
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
        if (stConfig.stFlasher[i].bEnabled)
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
        if (stConfig.stCounter[i].bEnabled)
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
        if (stConfig.stCondition[i].bEnabled)
            return true;
    }
    return false;
}

uint32_t GetConditions()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++) {
        result |= ((condition[i].nVal & 0x01) << i);
    }
    
    return result;
}

bool GetAnyKeypadEnable()
{
    for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
    {
        if (stConfig.stKeypad[i].bEnabled)
            return true;
    }
    return false;
}

bool GetKeypadEnable(uint8_t nKeypad)
{
    if (nKeypad >= PDM_NUM_KEYPADS)
        return false;

    return stConfig.stKeypad[nKeypad].bEnabled;
}

uint32_t GetKeypadButtons(uint8_t nKeypad)
{
    if (nKeypad >= PDM_NUM_KEYPADS)
        return 0;

    uint32_t result = 0;
    
    for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++) {
        result |= ((keypad[nKeypad].nVal[i] & 0x01) << i);
    }
    
    return result;
}

uint16_t GetKeypadDialVal(uint8_t nKeypad, uint8_t nDial)
{
    if (nKeypad >= PDM_NUM_KEYPADS)
        return 0;

    if (nDial >= KEYPAD_MAX_DIALS)
        return 0;

    return keypad[nKeypad].nVal[nDial];
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
    for(uint8_t i = 0; i < PDM_NUM_INPUTS; i++)
    {
        EnableLineEventWithPull(LINE_DI1, stConfig.stInput[i].ePull);
    }

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

