#include "config_handler.h"
#include "msg.h"
#include "device_config.h"
#include "can.h"
#include "can_input.h"
#include "can_outputs.h"
#include "counter.h"
#include "condition.h"
#include "flasher.h"
#include "virtual_input.h"
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

extern DeviceConfig stConfig;
extern CanInput canIn[NUM_CAN_INPUTS];
extern CanOutputs canOutputs;
extern VirtualInput virtIn[NUM_VIRT_INPUTS];
extern Flasher flasher[NUM_FLASHERS];
extern Counter counter[NUM_COUNTERS];
extern Condition condition[NUM_CONDITIONS];
#if NUM_OUTPUTS > 0
extern Profet pf[NUM_OUTPUTS];
#endif
#if HAS_WIPERS
extern Wiper wiper;
#endif
#if HAS_STARTER_DISABLE
extern Starter starter;
#endif
#if NUM_KEYPADS > 0
extern Keypad keypad[NUM_KEYPADS];
#endif
#if NUM_DIG_INPUTS > 0
extern Digital_Input digIn[NUM_DIG_INPUTS];
#endif
#if NUM_DIG_OUTPUTS > 0
extern Digital_Output digOut[NUM_DIG_OUTPUTS];
#endif
#if NUM_ANALOG_INPUTS > 0
extern Analog_Input analogIn[NUM_ANALOG_INPUTS];
#endif

void ApplyAllConfig()
{
    ApplyConfig(CanInput::nBaseIndex);
    ApplyConfig(CanOutputs::nBaseIndex);
    ApplyConfig(VirtualInput::nBaseIndex);
    ApplyConfig(Flasher::nBaseIndex);
    ApplyConfig(Counter::nBaseIndex);
    ApplyConfig(Condition::nBaseIndex);
    #if NUM_OUTPUTS > 0
    ApplyConfig(Profet::nBaseIndex);
    #endif
    #if HAS_WIPERS
    ApplyConfig(Wiper::nBaseIndex);
    #endif
    #if HAS_STARTER_DISABLE
    ApplyConfig(Starter::nBaseIndex);
    #endif
    #if NUM_KEYPADS > 0
    ApplyConfig(Keypad::nBaseIndex);
    #endif
    #if NUM_DIG_INPUTS > 0
    ApplyConfig(Digital_Input::nBaseIndex);
    #endif
    #if NUM_DIG_OUTPUTS > 0
    ApplyConfig(Digital_Output::nBaseIndex);
    #endif
    #if NUM_ANALOG_INPUTS > 0
    ApplyConfig(Analog_Input::nBaseIndex);
    #endif
}

void ApplyConfig(uint16_t nIndex)
{
    uint16_t nBaseIndex = nIndex & 0xFF00;

    // Device config (0x0000)
    if (nBaseIndex == 0x0000)
    {
        // TODO: Change CAN speed and filters without requiring reset
        
        SetCanFilterEnabled(stConfig.stDevice.bCanFilterEnabled);
    }

    if (nBaseIndex == CanInput::nBaseIndex)
    {
        ClearCanFilters(); // Clear all filters before setting new ones

        // Set filter for CAN settings request message, (Base ID - 1)
        // Use filter 0, it is always enabled to allow all messages by hal so it must be used
        SetCanFilterId(0, stConfig.stDevice.nBaseId - 1, false);

        for (uint8_t i = 0; i < NUM_CAN_INPUTS; i++)
        {
            canIn[i].SetConfig(&stConfig.stCanInput[i]);
            if(!stConfig.stCanInput[i].bEnabled)
                continue; // Skip if not enabled
            
            // Set filter for this input
            uint32_t nId = 0;
            nId = stConfig.stCanInput[i].nID;
            SetCanFilterId(i + 1, nId, stConfig.stCanInput[i].nIDE == 1);
        }

        //TODO: Set can filter without requiring reset, need a new message to indicate all IDs set before stopping CAN
    }

    if (nBaseIndex == CanOutputs::nBaseIndex)
    {
        canOutputs.SetConfig(stConfig.stCanOutput);

        CanOutputs::InitAllFrames();
    }

    if (nBaseIndex == VirtualInput::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_VIRT_INPUTS; i++)
            virtIn[i].SetConfig(&stConfig.stVirtualInput[i]);
    }

    if (nBaseIndex == Flasher::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_FLASHERS; i++)
            flasher[i].SetConfig(&stConfig.stFlasher[i]);
    }

    if (nBaseIndex == Counter::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_COUNTERS; i++)
            counter[i].SetConfig(&stConfig.stCounter[i]);
    }

    if (nBaseIndex == Condition::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_CONDITIONS; i++)
            condition[i].SetConfig(&stConfig.stCondition[i]);
    }

    #if NUM_OUTPUTS > 0
    if (nBaseIndex == Profet::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
            pf[i].SetConfig(&stConfig.stOutput[i]);

        // Clear all pairing pointers before linking
        for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
        {
            pf[i].pPrimary  = nullptr;
            pf[i].pFollower = nullptr;
        }

        // Link follower -> primary pairs with validation
        for (uint8_t i = 0; i < NUM_OUTPUTS; i++)
        {
            int8_t pri = stConfig.stOutput[i].nPrimaryOutput;

            if (pri == -1)                                           continue; // unpaired
            if (pri == i)                                            continue; // self-pair
            if (pri >= NUM_OUTPUTS)                                  continue; // out of range
            if (stConfig.stOutput[pri].nPrimaryOutput != -1)         continue; // primary is itself a follower (no chains)
            if (!stConfig.stOutput[pri].bEnabled)                    continue; // primary not enabled

            pf[i].pPrimary      = &pf[pri];
            pf[pri].pFollower   = &pf[i];
        }
    }
    #endif

    #if HAS_WIPERS
    if (nBaseIndex == Wiper::nBaseIndex)
    {
        wiper.SetConfig(&stConfig.stWiper);
    }
    #endif

    #if HAS_STARTER_DISABLE
    if (nBaseIndex == Starter::nBaseIndex)
    {
        starter.SetConfig(&stConfig.stStarter);
    }
    #endif

    #if NUM_KEYPADS > 0
    if (nBaseIndex == Keypad::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_KEYPADS; i++)
            keypad[i].SetConfig(&stConfig.stKeypad[i]);
    }
    #endif

    #if NUM_DIG_INPUTS > 0
    if (nBaseIndex == Digital_Input::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_DIG_INPUTS; i++)
            digIn[i].SetConfig(&stConfig.stDigInput[i]);
    }
    #endif

    #if NUM_DIG_OUTPUTS > 0
    if (nBaseIndex == Digital_Output::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_DIG_OUTPUTS; i++)
            digOut[i].SetConfig(&stConfig.stDigOutput[i]);
    }
    #endif

    #if NUM_ANALOG_INPUTS > 0
    if (nBaseIndex == Analog_Input::nBaseIndex)
    {
        for (uint8_t i = 0; i < NUM_ANALOG_INPUTS; i++)
            analogIn[i].SetConfig(&stConfig.stAnalogInput[i]);
    }
    #endif
}