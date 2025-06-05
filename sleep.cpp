#include "sleep.h"
#include "pdm.h"
#include "port.h"
#include "can.h"
#include "usb.h"
#include "status.h"
#include "mcu_utils.h"
#include "dingopdm_config.h"

// Static variables that were in pdm.cpp
static uint8_t nNumOutputsOn;
static uint8_t nLastNumOutputsOn;
static uint32_t nAllOutputsOffTime;
static bool bLastUsbConnected;
static uint32_t nUsbDisconnectedTime;

// External variables from pdm.cpp that we need access to
extern PdmConfig stConfig;
extern bool bSleepRequest;
extern Profet pf[PDM_NUM_OUTPUTS];

bool CheckEnterSleep()
{
    bool bEnterSleep = false;

    // Count number of outputs on
    nNumOutputsOn = 0;
    for (int i = 0; i < PDM_NUM_OUTPUTS; i++)
    {
        if (GetOutputState(i) != ProfetState::Off)
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

void EnableLineEventWithPull(ioline_t line, InputPull pull) 
{
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