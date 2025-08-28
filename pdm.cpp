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
#include "sleep.h"
#include "request_msg.h"
#include "infomsg.h"
#include "status.h"

// Component instances (global variables)
CanInput canIn[PDM_NUM_CAN_INPUTS];
VirtualInput virtIn[PDM_NUM_VIRT_INPUTS];
Wiper wiper;
Starter starter;
Flasher flasher[PDM_NUM_FLASHERS];
Counter counter[PDM_NUM_COUNTERS];
Condition condition[PDM_NUM_CONDITIONS];
Keypad keypad[PDM_NUM_KEYPADS];

// System state
PdmState eState = PdmState::Run;
FatalErrorType eError = FatalErrorType::NoError;
PdmConfig stConfig;
uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

// Monitoring variables
float fBattVolt;
float fTempSensor;
bool bDeviceOverTemp;
bool bDeviceCriticalTemp;
bool bSleepRequest;
bool bBootloaderRequest;

// Function declarations (private to this module)
void InitVarMap();
void CyclicUpdate();
void States();

// Thread definitions
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

    for(uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
        keypad[i].Init(i);

    InitInfoMsgs();

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

            for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
                keypad[i].CheckMsg(rxMsg);

            CheckRequestMsgs(&rxMsg);
            ApplyConfig(ConfigHandler(&rxMsg));
        }
    }

    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
        pf[i].Update(starter.nVal[i]);

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
    // Variable mapping stays in pdm.cpp due to tight coupling with all components
    // Numbers in comments below are only valid for dingoPDM/dingoPDM-Max
    // Other combinations of inputs/outputs will have different numbers

    // 0 - None - set to 0
    pVarMap[0] = const_cast<uint16_t*>(&ALWAYS_FALSE);

    // 1-2 - Digital inputs
    pVarMap[1] = &in[0].nVal;
    pVarMap[2] = &in[1].nVal;

    // 3-34 - CAN Inputs
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        pVarMap[i + 3] = &canIn[i].nOutput;
    }

    // 35-66 - CAN Input val
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++)
    {
        pVarMap[i + 35] = &canIn[i].nVal;
    }

    // 67-84 - Virtual Inputs
    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++)
    {
        pVarMap[i + 67] = &virtIn[i].nVal;
    }

    // 83-106 - Outputs
    for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        pVarMap[i * 3 + 83] = &pf[i].nOutput;
        pVarMap[i * 3 + 83 + 1] = &pf[i].nOvercurrent;
        pVarMap[i * 3 + 83 + 2] = &pf[i].nFault;
    }

    // 107-112 - Wiper
    pVarMap[107] = &wiper.nSlowOut;
    pVarMap[108] = &wiper.nFastOut;
    pVarMap[109] = &wiper.nParkOut;
    pVarMap[110] = &wiper.nInterOut;
    pVarMap[111] = &wiper.nWashOut;
    pVarMap[112] = &wiper.nSwipeOut;

    // 113-116 - Flashers
    for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
    {
        pVarMap[i + 113] = &flasher[i].nVal;
    }

    // 117-120 - Counters
    for (uint8_t i = 0; i < PDM_NUM_COUNTERS; i++)
    {
        pVarMap[i + 117] = &counter[i].nVal;
    }

    // 121-152 - Conditions
    for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
    {
        pVarMap[i + 121] = &condition[i].nVal;
    }

    // 153 - 192 - Keypad Buttons
    for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
    {
        for (uint8_t j = 0; j < KEYPAD_MAX_BUTTONS; j++)
        {
            pVarMap[i * KEYPAD_MAX_BUTTONS + j + 153] = &keypad[i].nVal[j];
        }
    }

    // 193 - 200 - Keypad Dials
    for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
    {
        for (uint8_t j = 0; j < KEYPAD_MAX_DIALS; j++)
        {
            pVarMap[i * KEYPAD_MAX_DIALS + j + 193] = &keypad[i].nDialVal[j];
        }
    }

    // 201 - Always true
    pVarMap[201] = const_cast<uint16_t*>(&ALWAYS_TRUE);

    //Note: Var map max size 255
    // 1 byte per var used in CAN messages
}