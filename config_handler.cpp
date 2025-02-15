#include "config_handler.h"
#include "msg.h"
#include "dingopdm_config.h"
#include "can_input.h"
#include "counter.h"
#include "condition.h"
#include "digital.h"
#include "flasher.h"
#include "profet.h"
#include "starter.h"
#include "virtual_input.h"
#include "wiper.h"

MsgCmd CanMsg(CANRxFrame *frame)
{
    // DLC 5 = Set CAN settings
    // DLC 1 = Get CAN settings

    if (frame->DLC == 5)
    {
        stConfig.stCanOutput.bEnabled = (frame->data8[1] & 0x02) >> 1;
        stConfig.stDevConfig.eCanSpeed = static_cast<CanBitrate>((frame->data8[1] & 0xF0) >> 4);
        stConfig.stCanOutput.nBaseId = (frame->data8[2] << 8) + frame->data8[3];
        stConfig.stCanOutput.nUpdateTime = frame->data8[4] * 10;
    }

    if ((frame->DLC == 5) ||
        (frame->DLC == 1))
    {
        CANTxFrame tx;
        tx.DLC = 5;
        tx.IDE = CAN_IDE_STD;

        tx.data8[0] = static_cast<uint8_t>(MsgCmd::Can) + 128;
        tx.data8[1] = ((static_cast<uint8_t>(stConfig.stDevConfig.eCanSpeed) & 0x0F) << 4) +
                      ((stConfig.stCanOutput.bEnabled & 0x01) << 1) + 1;
        tx.data8[2] = (stConfig.stCanOutput.nBaseId & 0xFF00) >> 8;
        tx.data8[3] = (stConfig.stCanOutput.nBaseId & 0x00FF);
        tx.data8[4] = (stConfig.stCanOutput.nUpdateTime) / 10;
        tx.data8[5] = 0;
        tx.data8[6] = 0;
        tx.data8[7] = 0;

        tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        PostTxFrame(&tx);

        if (frame->DLC == 5)
        {
            return MsgCmd::Can;
        }
    }
    return MsgCmd::Null;
}

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
            //res = Can::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Inputs:
            res = Digital::ProcessSettingsMsg(&stConfig, frame, &tx);
            break;
        case MsgCmd::Outputs:
            res = Profet::ProcessSettingsMsg(&stConfig, frame, &tx);
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
        default:
            break;
    }

    if (res != MsgCmdResult::Invalid)
    {
        PostTxFrame(&tx);
        //If MsgCmdResult::Request return Null to avoid applying new settings
        return (res == MsgCmdResult::Write) ? cmd : MsgCmd::Null;
    }
    return MsgCmd::Null;
}