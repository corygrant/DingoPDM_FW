#include "device.h"
#include "ch.hpp"
#include "hal.h"
#include "port.h"
#include "mcu_utils.h"
#include "device_config.h"
#include "config.h"
#include "param_protocol.h"
#include "config_handler.h"
#include "hw_devices.h"
#include "can.h"
#include "can_input.h"
#include "can_outputs.h"
#include "virtual_input.h"
#include "flasher.h"
#include "counter.h"
#include "condition.h"
#include "mailbox.h"
#include "msg.h"
#include "error.h"
#include "request_msg.h"
#include "infomsg.h"
#include "status.h"
#if HAS_WIPERS > 0
#include "wiper/wiper.h"
#endif
#if HAS_STARTER_DISABLE > 0
#include "starter.h"
#endif
#if HAS_USB
#include "usb.h"
#endif
#if NUM_KEYPADS > 0
#include "keypad/keypad.h"
#endif
#if CAN_SLEEP
#include "sleep.h"
#endif

CanInput canIn[NUM_CAN_INPUTS];
CanOutputs canOutputs;
VirtualInput virtIn[NUM_VIRT_INPUTS];
Flasher flasher[NUM_FLASHERS];
Counter counter[NUM_COUNTERS];
Condition condition[NUM_CONDITIONS];
#if HAS_WIPERS > 0
Wiper wiper;
#endif
#if HAS_STARTER_DISABLE > 0
Starter starter;
#endif
#if NUM_KEYPADS > 0
Keypad keypad[NUM_KEYPADS];
#endif

DeviceState eState = DeviceState::Run;
float fState; //For var map
FatalErrorType eError = FatalErrorType::NoError;
DeviceConfig stConfig;
DeviceConfig stConfigTemp; // Used for staging new config before applying
float *pVarMap[VAR_MAP_SIZE];

float fBattVolt;
float fTempSensor;
bool bDeviceOverTemp;
bool bDeviceCriticalTemp;
bool bSleepRequest;
bool bBootloaderRequest;

void InitVarMap();
void CyclicUpdate();
void States();

struct DeviceThread : chibios_rt::BaseStaticThread<DEVICE_THREAD_STACK>
{
    void main()
    {
        setName("DeviceThread");

        while (true)
        {
            CyclicUpdate();
            States();
            chThdSleepMilliseconds(2);
        }
    }
};
static DeviceThread deviceThread;

#if (HAS_BATT_VOLT_SENSE || HAS_EXT_TEMP_SENSOR)
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
#endif

void InitDevice()
{
    #if HAS_SE_LEDS
    Error::Initialize(&statusLed, &errorLed);
    #endif

    InitVarMap(); // Set val pointers

    #if HAS_I2C
    if (!i2cStart(&I2CD1, &i2cConfig) == HAL_RET_SUCCESS)
        Error::SetFatalError(FatalErrorType::ErrI2C, MsgSrc::Init);
    #endif

    InitConfig();

    ApplyAllConfig();

    if(!InitAdc() == HAL_RET_SUCCESS)
        Error::SetFatalError(FatalErrorType::ErrADC, MsgSrc::Init);
        
    if(!InitCan(&stConfig.stDevice) == HAL_RET_SUCCESS) // Starts CAN threads
        Error::SetFatalError(FatalErrorType::ErrCAN, MsgSrc::Init);

    #if HAS_USB
    if (!InitUsb() == HAL_RET_SUCCESS) // Starts USB threads
        Error::SetFatalError(FatalErrorType::ErrUSB, MsgSrc::Init);
    #endif

    #if HAS_EXT_TEMP_SENSOR
    if (!tempSensor.Init(BOARD_TEMP_WARN, BOARD_TEMP_CRIT))
        Error::SetFatalError(FatalErrorType::ErrTempSensor, MsgSrc::Init);
    #endif

    #if NUM_KEYPADS > 0
    Keypad::InitThread(keypad);
    #endif

    InitInfoMsgs();

    #if CAN_SLEEP
    palClearLine(LINE_CAN_STANDBY);
    #endif

    #if (HAS_BATT_VOLT_SENSE || HAS_EXT_TEMP_SENSOR)
    slowThreadRef = slowThread.start(NORMALPRIO);
    #endif

    deviceThread.start(NORMALPRIO);
}

void States()
{
    #if HAS_EXT_TEMP_SENSOR
    if (bDeviceCriticalTemp)
    {
        #if NUM_OUTPUTS > 0
        //Turn off all outputs
        for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
            pf[i].Update(false);
        #endif

        Error::SetFatalError(FatalErrorType::ErrTemp, MsgSrc::State_Overtemp);
    }
    #endif

    if (eState == DeviceState::Run)
    {
        #if HAS_EXT_TEMP_SENSOR
        if (bDeviceOverTemp)
            eState = DeviceState::OverTemp;
        #endif

        #if NUM_OUTPUTS > 0
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
        #endif
    }

    #if HAS_EXT_TEMP_SENSOR
    if (eState == DeviceState::OverTemp)
    {
        statusLed.Blink();
        errorLed.Blink();

        if (!bDeviceOverTemp)
            eState = DeviceState::Run;
    }
    #endif

    #if CAN_SLEEP
    if (CheckEnterSleep())
    {
        statusLed.Solid(false);
        errorLed.Solid(false);
        eState = DeviceState::Sleep;
    }

    if (eState == DeviceState::Sleep)
    {
        bSleepRequest = false;
        palSetLine(LINE_CAN_STANDBY); // CAN disabled
        EnterSleep();
    }
    #endif

    if (eState == DeviceState::Error)
    {
        // Not required?

        #if NUM_OUTPUTS > 0
        //Turn off all outputs
        for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
            pf[i].Update(false);
        #endif

        Error::SetFatalError(eError, MsgSrc::State_Error);
    }

    CheckInfoMsgs();

    fState = static_cast<float>(eState);
}

void CyclicUpdate()
{
    CANRxFrame rxMsg;

    while (!RxFramesEmpty())
    {
        msg_t res = FetchRxFrame(&rxMsg);
        if (res == MSG_OK)
        {
            for (uint8_t i = 0; i < NUM_CAN_INPUTS; i++)
                canIn[i].CheckMsg(rxMsg);

            #if NUM_KEYPADS > 0
            for (uint8_t i = 0; i < NUM_KEYPADS; i++)
                keypad[i].CheckMsg(rxMsg);
            #endif

            CheckRequestMsgs(&rxMsg);
            
            uint16_t nIndex = 0;
            MsgCmd cmd = ProcessParamMsg(&rxMsg, &nIndex);
            if (cmd == MsgCmd::WriteAllComplete)
            {
                ApplyAllConfig();
            }
            if (cmd == MsgCmd::Write)
            {
                ApplyConfig(nIndex & 0xFF00); // Mask instance, only base index is needed
            }
        }
    }

    #if NUM_OUTPUTS > 0
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
        pf[i].Update(starter.fVal[i]);
    #endif

    #if NUM_DIG_INPUTS > 0
    for (uint8_t i = 0; i < NUM_DIG_INPUTS; i++)
        digIn[i].Update();
    #endif

    #if NUM_DIG_OUTPUTS > 0
    for (uint8_t i = 0; i < NUM_DIG_OUTPUTS; i++)
        digOut[i].Update();
    #endif

    #if NUM_ANALOG_INPUTS > 0
    for (uint8_t i = 0; i < NUM_ANALOG_INPUTS; i++)
        analogIn[i].Update();
    #endif

    #if NUM_CAN_INPUTS > 0
    for (uint8_t i = 0; i < NUM_CAN_INPUTS; i++)
        canIn[i].CheckTimeout();
    #endif

    #if NUM_CAN_OUTPUTS > 0
    canOutputs.Update();
    #endif

    #if NUM_VIRT_INPUTS > 0
    for (uint8_t i = 0; i < NUM_VIRT_INPUTS; i++)
        virtIn[i].Update();
    #endif

    #if HAS_WIPERS
    wiper.Update();
    #endif

    #if HAS_STARTER_DISABLE
    starter.Update();
    #endif

    #if NUM_FLASHERS > 0
    for (uint8_t i = 0; i < NUM_FLASHERS; i++)
        flasher[i].Update(SYS_TIME);
    #endif

    #if NUM_COUNTERS > 0
    for (uint8_t i = 0; i < NUM_COUNTERS; i++)
        counter[i].Update();
    #endif

    #if NUM_CONDITIONS > 0    
    for (uint8_t i = 0; i < NUM_CONDITIONS; i++)
        condition[i].Update();
    #endif

    #if NUM_KEYPADS > 0
    for (uint8_t i = 0; i < NUM_KEYPADS; i++)
        keypad[i].Update();
    #endif

    #if HAS_NEOPIXELS
    UpdateNeopixels();
    #endif
}

void InitVarMap()
{
    uint16_t index = 0;
    
    //System vars
    pVarMap[index++] = const_cast<float*>(&ALWAYS_FALSE);
    pVarMap[index++] = const_cast<float*>(&ALWAYS_TRUE);
    pVarMap[index++] = &fState; 
    #if HAS_EXT_TEMP_SENSOR
    pVarMap[index++] = &fTempSensor;
    #endif
    #if HAS_BATT_VOLT_SENSE
    pVarMap[index++] = &fBattVolt;
    #endif

    #if NUM_DIG_INPUTS > 0
    for (uint8_t i = 0; i < NUM_DIG_INPUTS; i++)
        pVarMap[index++] = &digIn[i].fVal;
    #endif

    #if NUM_DIG_OUTPUTS > 0
    for (uint8_t i = 0; i < NUM_DIG_OUTPUTS; i++)
        pVarMap[index++] = &digOut[i].fVal;
    #endif

    #if NUM_ANALOG_INPUTS > 0
    for (uint8_t i = 0; i < NUM_ANALOG_INPUTS; i++)
    {
        pVarMap[index++] = &analogIn[i].fVal;
        pVarMap[index++] = &analogIn[i].fValMillivolts;
        pVarMap[index++] = &analogIn[i].fRotaryPos;
        pVarMap[index++] = &analogIn[i].fSwitchVal;
    }
    #endif

    #if NUM_CAN_INPUTS > 0
    for (uint8_t i = 0; i < NUM_CAN_INPUTS; i++)
    {
        pVarMap[index++] = &canIn[i].fOutput;
        pVarMap[index++] = &canIn[i].fVal;
    }
    #endif

    #if NUM_VIRT_INPUTS > 0
    for (uint8_t i = 0; i < NUM_VIRT_INPUTS; i++)
    {
        pVarMap[index++] = &virtIn[i].fVal;
    }
    #endif

    #if NUM_OUTPUTS > 0
    // Outputs
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
    {
        pVarMap[index++] = &pf[i].fOutput;
        pVarMap[index++] = &pf[i].fCurrent;
        pVarMap[index++] = &pf[i].fOvercurrent;
        pVarMap[index++] = &pf[i].fFault;
    }
    #endif

    #if NUM_FLASHERS > 0
    // Flashers
    for (uint8_t i = 0; i < NUM_FLASHERS; i++)
    {
        pVarMap[index++] = &flasher[i].fVal;
    }
    #endif

    #if NUM_CONDITIONS > 0
    // Conditions
    for (uint8_t i = 0; i < NUM_CONDITIONS; i++)
    {
        pVarMap[index++] = &condition[i].fVal;
    }
    #endif

    #if NUM_COUNTERS > 0
    // Counters
    for (uint8_t i = 0; i < NUM_COUNTERS; i++)
    {
        pVarMap[index++] = &counter[i].fVal;
    }
    #endif

    #if HAS_WIPERS > 0
    // Wiper
    pVarMap[index++] = &wiper.fSlowOut;
    pVarMap[index++] = &wiper.fFastOut;
    pVarMap[index++] = &wiper.fParkOut;
    pVarMap[index++] = &wiper.fInterOut;
    pVarMap[index++] = &wiper.fWashOut;
    pVarMap[index++] = &wiper.fSwipeOut;
    #endif

    #if NUM_KEYPADS > 0
    // Keypads
    for (uint8_t i = 0; i < NUM_KEYPADS; i++)
    {
        for (uint8_t j = 0; j < KEYPAD_MAX_BUTTONS; j++)
        {
            pVarMap[index++] = &keypad[i].fButtonVal[j];
        }

        for (uint8_t j = 0; j < KEYPAD_MAX_DIALS; j++)
        {
            pVarMap[index++] = &keypad[i].fDialVal[j];
        }

        for (uint8_t j = 0; j < KEYPAD_MAX_ANALOG_INPUTS; j++)
        {
            pVarMap[index++] = &keypad[i].fAnalogVal[j];
        }
    }
    #endif

    //VarMap size must match the expected size
    if (index != VAR_MAP_SIZE)
        Error::SetFatalError(FatalErrorType::ErrVarMap, MsgSrc::Init);

}