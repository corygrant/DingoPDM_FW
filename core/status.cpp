#include "status.h"
#include "pdm.h"
#include "config.h"
#include "dingopdm_config.h"
#include "profet.h"
#include "digital.h"
#include "can_input.h"
#include "virtual_input.h"
#include "wiper/wiper.h"
#include "starter.h"
#include "flasher.h"
#include "counter.h"
#include "condition.h"
#include "keypad/keypad.h"

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

    return in[nInput].fVal;
}

float GetOutputCurrent(uint8_t nOutput)
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

    return canIn[nInput].fOutput;
}

float GetCanInVal(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return false;

    return canIn[nInput].fVal;
}

float GetCanInFactor(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return 0;

    return stConfig.stCanInput[nInput].fFactor;
}

float GetCanInOffset(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return 0;

    return stConfig.stCanInput[nInput].fOffset;
}

ByteOrder GetCanInByteOrder(uint8_t nInput)
{
    if (nInput >= PDM_NUM_CAN_INPUTS)
        return ByteOrder::LittleEndian;

    return stConfig.stCanInput[nInput].eByteOrder;
}

uint32_t GetCanInOutputs()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < PDM_NUM_CAN_INPUTS; i++) {
        result |= (((uint32_t)canIn[i].fVal & 0x01) << i);
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

bool GetVirtInVal(uint8_t nInput)
{
    if (nInput >= PDM_NUM_VIRT_INPUTS)
        return false;

    return virtIn[nInput].fVal;
}

uint32_t GetVirtIns()
{
    uint32_t result = 0;
    
    for (uint8_t i = 0; i < PDM_NUM_VIRT_INPUTS; i++) {
        result |= (((uint32_t)virtIn[i].fVal & 0x01) << i);
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

    return flasher[nFlasher].fVal;
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

float GetCounterVal(uint8_t nCounter)
{
    if (nCounter >= PDM_NUM_COUNTERS)
        return 0;

    return counter[nCounter].fVal;
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
        result |= (((uint32_t)condition[i].fVal & 0x01) << i);
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
        result |= (((uint32_t)keypad[nKeypad].fVal[i] & 0x01) << i);
    }
    
    return result;
}

float GetKeypadDialVal(uint8_t nKeypad, uint8_t nDial)
{
    if (nKeypad >= PDM_NUM_KEYPADS)
        return 0;

    if (nDial >= KEYPAD_MAX_DIALS)
        return 0;

    return keypad[nKeypad].nDialVal[nDial];
}