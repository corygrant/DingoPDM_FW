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