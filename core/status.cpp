#include "status.h"
#include "device.h"
#include "config.h"
#include "device_config.h"
#include "can_input.h"
#include "virtual_input.h"
#include "flasher.h"
#include "counter.h"
#include "condition.h"
#if NUM_OUTPUTS > 0
#include "profet.h"
#endif
#if HAS_WIPERS > 0
#include "wiper/wiper.h"
#endif
#if HAS_STARTER_DISABLE > 0
#include "starter.h"
#endif
#if NUM_KEYPADS > 0
#include "keypad/keypad.h"
#endif
#if NUM_DIG_INPUTS > 0
#include "digital_input.h"
#endif
#if NUM_DIG_OUTPUTS > 0
#include "digital_output.h"
#endif
#if NUM_ANALOG_INPUTS > 0
#include "analog_input.h"
#endif

DeviceState GetDeviceState()
{
    return eState;
}

#if HAS_EXT_TEMP_SENSOR
float GetBoardTemp()
{
    return fTempSensor;
}
#endif

#if NUM_OUTPUTS > 0
float GetTotalCurrent()
{
    float fTotalCurrent = 0.0;

    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
    {
        if (pf[i].pPrimary != nullptr)
            continue; // Skip followers since their current is included in the primary
        fTotalCurrent += pf[i].GetCurrent();
    }

    return fTotalCurrent;
}

bool GetAnyOvercurrent()
{
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
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
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
    {
        if (pf[i].GetState() == ProfetState::Fault)
        {
            return true;
        }
    }

    return false;
}

float GetOutputCurrent(uint8_t nOutput)
{
    if (nOutput >= NUM_OUTPUTS)
        return 0;

    return pf[nOutput].GetCurrent();
}

ProfetState GetOutputState(uint8_t nOutput)
{
    if (nOutput >= NUM_OUTPUTS)
        return ProfetState::Off;

    return pf[nOutput].GetState();
}

uint8_t GetOutputOcCount(uint8_t nOutput)
{
    if (nOutput >= NUM_OUTPUTS)
        return 0;

    return pf[nOutput].GetOcCount();
}

uint8_t GetOutputDC(uint8_t nOutput)
{
    if (nOutput >= NUM_OUTPUTS)
        return 0;

    return pf[nOutput].GetDutyCycle();
}

bool GetAnyPwmEnable()
{
    for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
    {
        if (stConfig.stOutput[i].stPwm.bEnabled)
            return true;
    }
    return false;
}
#endif

#if NUM_DIG_INPUTS > 0
bool GetDigInputVal(uint8_t nInput)
{
    if (nInput >= NUM_DIG_INPUTS)
        return false;

    return digIn[nInput].fVal;
}
#endif

bool GetAnyCanInEnable()
{
    for (uint8_t i = 0; i < NUM_CAN_INPUTS; i++)
    {
        if (stConfig.stCanInput[i].bEnabled)
            return true;
    }
    return false;
}

bool GetCanInEnable(uint8_t nInput)
{
    if (nInput >= NUM_CAN_INPUTS)
        return false;

    return stConfig.stCanInput[nInput].bEnabled;
}

bool GetCanInOutput(uint8_t nInput)
{
    if (nInput >= NUM_CAN_INPUTS)
        return false;

    return canIn[nInput].fOutput;
}

float GetCanInVal(uint8_t nInput)
{
    if (nInput >= NUM_CAN_INPUTS)
        return false;

    return canIn[nInput].fVal;
}

float GetCanInFactor(uint8_t nInput)
{
    if (nInput >= NUM_CAN_INPUTS)
        return 0;

    return stConfig.stCanInput[nInput].fFactor;
}

float GetCanInOffset(uint8_t nInput)
{
    if (nInput >= NUM_CAN_INPUTS)
        return 0;

    return stConfig.stCanInput[nInput].fOffset;
}

ByteOrder GetCanInByteOrder(uint8_t nInput)
{
    if (nInput >= NUM_CAN_INPUTS)
        return ByteOrder::LittleEndian;

    return stConfig.stCanInput[nInput].eByteOrder;
}

uint32_t GetCanInOutputs()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < NUM_CAN_INPUTS; i++) {
        result |= (((uint32_t)canIn[i].fVal & 0x01) << i);
    }
    
    return result;
}

bool GetAnyVirtInEnable()
{
    for (uint8_t i = 0; i < NUM_VIRT_INPUTS; i++)
    {
        if (stConfig.stVirtualInput[i].bEnabled)
            return true;
    }
    return false;
}

bool GetVirtInVal(uint8_t nInput)
{
    if (nInput >= NUM_VIRT_INPUTS)
        return false;

    return virtIn[nInput].fVal;
}

uint32_t GetVirtIns()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < NUM_VIRT_INPUTS; i++) {
        result |= (((uint32_t)virtIn[i].fVal & 0x01) << i);
    }
    
    return result;
}

#if HAS_WIPERS
bool GetWiperEnable()
{
    return stConfig.stWiper.bEnabled;
}

bool GetWiperFastOut()
{
    return wiper.fFastOut > 0.0f;
}

bool GetWiperSlowOut()
{
    return wiper.fSlowOut > 0.0f;
}

WiperState GetWiperState()
{
    return wiper.GetState();
}

WiperSpeed GetWiperSpeed()
{
    return wiper.GetSpeed();
}
#endif

bool GetAnyFlasherEnable()
{
    for (uint8_t i = 0; i < NUM_FLASHERS; i++)
    {
        if (stConfig.stFlasher[i].bEnabled)
            return true;
    }
    return false;
}

bool GetFlasherVal(uint8_t nFlasher)
{
    if (nFlasher >= NUM_FLASHERS)
        return false;

    return flasher[nFlasher].fVal;
}

bool GetAnyCounterEnable()
{
    for (uint8_t i = 0; i < NUM_COUNTERS; i++)
    {
        if (stConfig.stCounter[i].bEnabled)
            return true;
    }
    return false;
}

float GetCounterVal(uint8_t nCounter)
{
    if (nCounter >= NUM_COUNTERS)
        return 0;

    return counter[nCounter].fVal;
}

bool GetAnyConditionEnable()
{
    for (uint8_t i = 0; i < NUM_CONDITIONS; i++)
    {
        if (stConfig.stCondition[i].bEnabled)
            return true;
    }
    return false;
}

uint32_t GetConditions()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < NUM_CONDITIONS; i++) {
        result |= (((uint32_t)condition[i].fVal & 0x01) << i);
    }
    
    return result;
}

#if NUM_KEYPADS > 0
bool GetAnyKeypadEnable()
{
    #if NUM_KEYPADS > 0
    for (uint8_t i = 0; i < NUM_KEYPADS; i++)
    {
        if (stConfig.stKeypad[i].bEnabled)
            return true;
    }
    #endif
    return false;
}

bool GetKeypadEnable(uint8_t nKeypad)
{
    if (nKeypad >= NUM_KEYPADS)
        return false;

    #if NUM_KEYPADS > 0
    return stConfig.stKeypad[nKeypad].bEnabled;
    #else
    return false;
    #endif
}

uint32_t GetKeypadButtons(uint8_t nKeypad)
{
    if (nKeypad >= NUM_KEYPADS)
        return 0;

    #if NUM_KEYPADS > 0
    uint32_t result = 0;

    for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++) {
        result |= (((uint32_t)keypad[nKeypad].fButtonVal[i] & 0x01) << i);
    }

    return result;
    #else
    return 0;
    #endif
}

float GetKeypadDialVal(uint8_t nKeypad, uint8_t nDial)
{
    if (nKeypad >= NUM_KEYPADS)
        return 0;

    if (nDial >= KEYPAD_MAX_DIALS)
        return 0;

    #if NUM_KEYPADS > 0
    return keypad[nKeypad].fDialVal[nDial];
    #else
    return 0;
    #endif
}
#endif

#if NUM_DIG_OUTPUTS > 0
bool GetDigOutputState(uint8_t nOutput)
{
    if (nOutput >= NUM_DIG_OUTPUTS)
        return false;

    return static_cast<bool>(digOut[nOutput].fVal);
}
#endif

#if NUM_ANALOG_INPUTS > 0
uint16_t GetAnalogInputVal(uint8_t nInput)
{
    if (nInput >= NUM_ANALOG_INPUTS)
        return 0;

    return static_cast<uint16_t>(analogIn[nInput].fVal);
}

float GetAnalogInputMv(uint8_t nInput)
{
    if (nInput >= NUM_ANALOG_INPUTS)
        return 0;

    return analogIn[nInput].fValMillivolts;
}

uint8_t GetRotarySwitchPos(uint8_t nInput)
{
    if (nInput >= NUM_ANALOG_INPUTS)
        return 0;

    return static_cast<uint8_t>(analogIn[nInput].fRotaryPos);
}

bool GetAnalogSwitchVal(uint8_t nInput)
{
    if (nInput >= NUM_ANALOG_INPUTS)
        return false;

    return static_cast<bool>(analogIn[nInput].fSwitchVal);
}

bool GetAnyAnalogInputEnable()
{
    for (uint8_t i = 0; i < NUM_ANALOG_INPUTS; i++)
    {
        if (stConfig.stAnalogInput[i].bEnabled)
            return true;
    }
    return false;
}
#endif