#include "config_handler.h"
#include "msg.h"
#include "dingopdm_config.h"
#include "can.h"
#include "can_input.h"
#include "counter.h"
#include "condition.h"
#include "digital.h"
#include "flasher.h"
#include "profet.h"
#include "starter.h"
#include "virtual_input.h"
#include "wiper/wiper.h"
#include "keypad/keypad.h"

MsgCmd ConfigHandler(CANRxFrame *frame)
{
    CANTxFrame tx;
    MsgCmd cmd;
    MsgCmdResult res = MsgCmdResult::Invalid;

    if (frame->SID != stConfig.stCanOutput.nBaseId - 1)
    {
        return MsgCmd::Null;
    }

    cmd = static_cast<MsgCmd>(frame->data8[0]);

    switch(cmd)
    {
        case MsgCmd::Can:
            res = CanProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Inputs:
            res = Digital::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Outputs:
            res = Profet::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::OutputsPwm:
            res = Pwm::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::CanInputs:
        case MsgCmd::CanInputsId:
            res = CanInput::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::VirtualInputs:
            res = VirtualInput::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Wiper:
        case MsgCmd::WiperSpeed:
        case MsgCmd::WiperDelays:
            res = Wiper::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Flashers:
            res = Flasher::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::StarterDisable:
            res = Starter::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Counters:
            res = Counter::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Conditions:
            res = Condition::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Keypad:
        case MsgCmd::KeypadLed:
        case MsgCmd::KeypadButton:
        case MsgCmd::KeypadButtonLed:
        case MsgCmd::KeypadDial:
            res = Keypad::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        default:
            break;
    }

    if (res != MsgCmdResult::Invalid)
    {
        tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        PostTxFrame(&tx);
        //If MsgCmdResult::Request return Null to avoid applying new settings
        return (res == MsgCmdResult::Write) ? cmd : MsgCmd::Null;
    }
    return MsgCmd::Null;
}

extern PdmConfig stConfig;
extern float *pVarMap[PDM_VAR_MAP_SIZE];
extern Digital in[PDM_NUM_INPUTS];
extern CanInput canIn[PDM_NUM_CAN_INPUTS];
extern VirtualInput virtIn[PDM_NUM_VIRT_INPUTS];
extern Profet pf[PDM_NUM_OUTPUTS];
extern Wiper wiper;
extern Starter starter;
extern Flasher flasher[PDM_NUM_FLASHERS];
extern Counter counter[PDM_NUM_COUNTERS];
extern Condition condition[PDM_NUM_CONDITIONS];
extern Keypad keypad[PDM_NUM_KEYPADS];

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
            virtIn[i].SetConfig(&stConfig.stVirtualInput[i]);
    }

    if (eCmd == MsgCmd::Outputs)
    {
        for (uint8_t i = 0; i < PDM_NUM_OUTPUTS; i++)
            pf[i].SetConfig(&stConfig.stOutput[i]);
    }

    if ((eCmd == MsgCmd::Wiper) || (eCmd == MsgCmd::WiperSpeed) || (eCmd == MsgCmd::WiperDelays))
    {
        wiper.SetConfig(&stConfig.stWiper);
    }

    if (eCmd == MsgCmd::StarterDisable)
    {
        starter.SetConfig(&stConfig.stStarter);
    }

    if (eCmd == MsgCmd::Flashers)
    {
        for (uint8_t i = 0; i < PDM_NUM_FLASHERS; i++)
            flasher[i].SetConfig(&stConfig.stFlasher[i]);
    }

    if (eCmd == MsgCmd::Counters)
    {
        for (uint8_t i = 0; i < PDM_NUM_COUNTERS; i++)
            counter[i].SetConfig(&stConfig.stCounter[i]);
    }

    if (eCmd == MsgCmd::Conditions)
    {
        for (uint8_t i = 0; i < PDM_NUM_CONDITIONS; i++)
            condition[i].SetConfig(&stConfig.stCondition[i]);
    }

    if ((eCmd == MsgCmd::Keypad) || (eCmd == MsgCmd::KeypadLed) || (eCmd == MsgCmd::KeypadButton) ||
        (eCmd == MsgCmd::KeypadButtonLed) || (eCmd == MsgCmd::KeypadDial))
    {
        for (uint8_t i = 0; i < PDM_NUM_KEYPADS; i++)
            keypad[i].SetConfig(&stConfig.stKeypad[i]);
    }
}