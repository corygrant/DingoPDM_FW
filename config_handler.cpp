#include "config_handler.h"
#include "msg.h"
#include "dingopdm_config.h"    

MsgCmdRx CanMsg(CANRxFrame *frame)
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

        tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::Can);
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

        if(frame->DLC == 5)
        {
            return MsgCmdRx::Can;
        }
    }
    return MsgCmdRx::Null;
}

MsgCmdRx InputMsg(CANRxFrame *frame)
{
    // DLC 4 = Set input settings
    // DLC 2 = Get input settings

    if ((frame->DLC == 4) ||
        (frame->DLC == 2))
    {
        uint8_t nIndex = (frame->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_INPUTS)
        {

            if (frame->DLC == 4)
            {
                stConfig.stInput[nIndex].bEnabled = (frame->data8[1] & 0x01);
                stConfig.stInput[nIndex].eMode = static_cast<InputMode>((frame->data8[1] & 0x06) >> 1);
                stConfig.stInput[nIndex].bInvert = (frame->data8[1] & 0x08) >> 3;
                stConfig.stInput[nIndex].nDebounceTime = frame->data8[2] * 10;
                stConfig.stInput[nIndex].ePull = static_cast<InputPull>(frame->data8[3] & 0x03);
            }

            CANTxFrame tx;
            tx.DLC = 4;
            tx.IDE = CAN_IDE_STD;

            tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::Inputs);
            tx.data8[1] = ((nIndex & 0x0F) << 4) + ((stConfig.stInput[nIndex].bInvert & 0x01) << 3) +
                          ((static_cast<uint8_t>(stConfig.stInput[nIndex].eMode) & 0x03) << 1) + (stConfig.stInput[nIndex].bEnabled & 0x01);
            tx.data8[2] = (uint8_t)(stConfig.stInput[nIndex].nDebounceTime / 10);
            tx.data8[3] = static_cast<uint8_t>(stConfig.stInput[nIndex].ePull);
            tx.data8[4] = 0;
            tx.data8[5] = 0;
            tx.data8[6] = 0;
            tx.data8[7] = 0;

            tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            PostTxFrame(&tx);
            
            if(frame->DLC == 4)
                return MsgCmdRx::Inputs;
        }
    }

    return MsgCmdRx::Null;
}

MsgCmdRx OutputMsg(CANRxFrame *frame)
{
    // DLC 8 = Set output settings
    // DLC 2 = Get output settings

    if ((frame->DLC == 8) ||
        (frame->DLC == 2))
    {
        uint8_t nIndex = (frame->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_OUTPUTS)
        {
            if (frame->DLC == 8)
            {
                stConfig.stOutput[nIndex].bEnabled = (frame->data8[1] & 0x01);
                stConfig.stOutput[nIndex].nInput = frame->data8[2];
                stConfig.stOutput[nIndex].nCurrentLimit = frame->data8[3] * 10;
                stConfig.stOutput[nIndex].eResetMode = static_cast<ProfetResetMode>(frame->data8[4] & 0x0F);
                stConfig.stOutput[nIndex].nResetLimit = (frame->data8[4] & 0xF0) >> 4;
                stConfig.stOutput[nIndex].nResetTime = frame->data8[5] * 100;
                stConfig.stOutput[nIndex].nInrushLimit = frame->data8[6] * 10;
                stConfig.stOutput[nIndex].nInrushTime = frame->data8[7] * 100;
            }

            CANTxFrame tx;
            tx.DLC = 8;
            tx.IDE = CAN_IDE_STD;

            tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::Outputs);
            tx.data8[1] = ((nIndex & 0x0F) << 4) + (stConfig.stOutput[nIndex].bEnabled & 0x01);
            tx.data8[2] = stConfig.stOutput[nIndex].nInput;
            tx.data8[3] = (uint8_t)(stConfig.stOutput[nIndex].nCurrentLimit / 10);
            tx.data8[4] = ((stConfig.stOutput[nIndex].nResetLimit & 0x0F) << 4) +
                          (static_cast<uint8_t>(stConfig.stOutput[nIndex].eResetMode) & 0x0F);
            tx.data8[5] = (uint8_t)(stConfig.stOutput[nIndex].nResetTime / 100);
            tx.data8[6] = (uint8_t)(stConfig.stOutput[nIndex].nInrushLimit / 10);
            tx.data8[7] = (uint8_t)(stConfig.stOutput[nIndex].nInrushTime / 100);

            tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            PostTxFrame(&tx);

            if(frame->DLC == 8)
                return MsgCmdRx::Outputs;
        }
    }

    return MsgCmdRx::Null;
}

MsgCmdRx CanInputMsg(CANRxFrame *frame)
{
    // DLC 7 = Set CAN input settings
    // DLC 2 = Get CAN input settings

    if ((frame->DLC == 7) ||
        (frame->DLC == 2))
    {
        uint8_t nIndex;
        if (frame->DLC == 7)
            nIndex = frame->data8[2];
        else
            nIndex = frame->data8[1];

        if (nIndex < PDM_NUM_CAN_INPUTS)
        {
            if (frame->DLC == 7)
            {
                stConfig.stCanInput[nIndex].bEnabled = (frame->data8[1] & 0x01);
                stConfig.stCanInput[nIndex].eMode = static_cast<InputMode>((frame->data8[1] & 0x06) >> 1);
                stConfig.stCanInput[nIndex].eOperator = static_cast<Operator>((frame->data8[1] & 0xF0) >> 4);

                stConfig.stCanInput[nIndex].nSID = (frame->data8[3] << 8) + frame->data8[4];

                stConfig.stCanInput[nIndex].nStartingByte = (frame->data8[5] & 0x0F);
                stConfig.stCanInput[nIndex].nDLC = (frame->data8[5] & 0xF0) >> 4;

                stConfig.stCanInput[nIndex].nOnVal = frame->data8[6];
            }

            CANTxFrame tx;
            tx.DLC = 7;
            tx.IDE = CAN_IDE_STD;

            tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::CanInputs);
            tx.data8[1] = ((static_cast<uint8_t>(stConfig.stCanInput[nIndex].eOperator) & 0x0F) << 4) +
                          ((static_cast<uint8_t>(stConfig.stCanInput[nIndex].eMode) & 0x03) << 1) +
                          (stConfig.stCanInput[nIndex].bEnabled & 0x01);
            tx.data8[2] = nIndex;
            tx.data8[3] = (uint8_t)(stConfig.stCanInput[nIndex].nSID >> 8);
            tx.data8[4] = (uint8_t)(stConfig.stCanInput[nIndex].nSID & 0xFF);
            tx.data8[5] = ((stConfig.stCanInput[nIndex].nDLC & 0xF) << 4) +
                          (stConfig.stCanInput[nIndex].nStartingByte & 0xF);
            tx.data8[6] = (uint8_t)(stConfig.stCanInput[nIndex].nOnVal);

            tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            PostTxFrame(&tx);

            if(frame->DLC == 7)
                return MsgCmdRx::CanInputs;
        }
    }

    return MsgCmdRx::Null;
}

MsgCmdRx VirtualInputMsg(CANRxFrame *frame)
{
    // DLC 7 = Set virtual input settings
    // DLC 2 = Get virtual input settings

    if ((frame->DLC == 7) ||
        (frame->DLC == 2))
    {
        uint8_t nIndex;
        if (frame->DLC == 7)
            nIndex = frame->data8[2];
        else
            nIndex = frame->data8[1];

        if (nIndex < PDM_NUM_VIRT_INPUTS)
        {
            if (frame->DLC == 7)
            {
                stConfig.stVirtualInput[nIndex].bEnabled = (frame->data8[1] & 0x01);
                stConfig.stVirtualInput[nIndex].bNot0 = (frame->data8[1] & 0x02) >> 1;
                stConfig.stVirtualInput[nIndex].bNot1 = (frame->data8[1] & 0x04) >> 2;
                stConfig.stVirtualInput[nIndex].bNot2 = (frame->data8[1] & 0x08) >> 3;
                stConfig.stVirtualInput[nIndex].nVar0 = frame->data8[3];
                stConfig.stVirtualInput[nIndex].nVar1 = frame->data8[4];
                stConfig.stVirtualInput[nIndex].nVar2 = frame->data8[5];
                stConfig.stVirtualInput[nIndex].eCond0 = static_cast<Condition>((frame->data8[6] & 0x03));
                stConfig.stVirtualInput[nIndex].eCond1 = static_cast<Condition>((frame->data8[6] & 0x0C) >> 2);
                stConfig.stVirtualInput[nIndex].eMode = static_cast<InputMode>((frame->data8[6] & 0xC0) >> 6);
            }

            CANTxFrame tx;
            tx.DLC = 7;
            tx.IDE = CAN_IDE_STD;

            tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::VirtualInputs);
            tx.data8[1] = ((stConfig.stVirtualInput[nIndex].bNot2 & 0x01) << 3) +
                          ((stConfig.stVirtualInput[nIndex].bNot1 & 0x01) << 2) +
                          ((stConfig.stVirtualInput[nIndex].bNot0 & 0x01) << 1) +
                          (stConfig.stVirtualInput[nIndex].bEnabled & 0x01);
            tx.data8[2] = nIndex;
            tx.data8[3] = stConfig.stVirtualInput[nIndex].nVar0;
            tx.data8[4] = stConfig.stVirtualInput[nIndex].nVar1;
            tx.data8[5] = stConfig.stVirtualInput[nIndex].nVar2;
            tx.data8[6] = ((static_cast<uint8_t>(stConfig.stVirtualInput[nIndex].eMode) & 0x0F) << 6) +
                          ((static_cast<uint8_t>(stConfig.stVirtualInput[nIndex].eCond1) & 0x03) << 2) +
                          (static_cast<uint8_t>(stConfig.stVirtualInput[nIndex].eCond0) & 0x03);
            tx.data8[7] = 0;

            tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            PostTxFrame(&tx);

            if(frame->DLC == 7)
                return MsgCmdRx::VirtualInputs;
        }
    }

    return MsgCmdRx::Null;
}

MsgCmdRx WiperMsg(CANRxFrame *frame)
{
    // DLC 8 = Set wiper settings
    // DLC 1 = Get wiper settings

    if ((frame->DLC == 8) ||
        (frame->DLC == 1))
    {
        if (frame->DLC == 8)
        {
            stConfig.stWiper.bEnabled = (frame->data8[1] & 0x01);
            stConfig.stWiper.eMode = static_cast<WiperMode>((frame->data8[1] & 0x06) >> 1);
            stConfig.stWiper.bParkStopLevel = (frame->data8[1] & 0x08) >> 3;
            stConfig.stWiper.nWashWipeCycles = (frame->data8[1] & 0xF0) >> 4;
            stConfig.stWiper.nSlowInput = frame->data8[2];
            stConfig.stWiper.nFastInput = frame->data8[3];
            stConfig.stWiper.nInterInput = frame->data8[4];
            stConfig.stWiper.nOnInput = frame->data8[5];
            stConfig.stWiper.nParkInput = frame->data8[6];
            stConfig.stWiper.nWashInput = frame->data8[7];
        }

        CANTxFrame tx;
        tx.DLC = 8;
        tx.IDE = CAN_IDE_STD;

        tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::Wiper);
        tx.data8[1] = ((stConfig.stWiper.nWashWipeCycles & 0x0F) << 4) +
                      ((stConfig.stWiper.bParkStopLevel & 0x01) << 3) +
                      ((static_cast<uint8_t>(stConfig.stWiper.eMode) & 0x03) << 2) +
                      (stConfig.stWiper.bEnabled & 0x01);
        tx.data8[2] = stConfig.stWiper.nSlowInput;
        tx.data8[3] = stConfig.stWiper.nFastInput;
        tx.data8[4] = stConfig.stWiper.nInterInput;
        tx.data8[5] = stConfig.stWiper.nOnInput;
        tx.data8[6] = stConfig.stWiper.nParkInput;
        tx.data8[7] = stConfig.stWiper.nWashInput;

        tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        PostTxFrame(&tx);

        if(frame->DLC == 8)
            return MsgCmdRx::Wiper;
    }

    return MsgCmdRx::Null;
}

MsgCmdRx WiperSpeedMsg(CANRxFrame *frame)
{
    // DLC 7 = Set wiper speed settings
    // DLC 1 = Get wiper speed settings

    if ((frame->DLC == 7) ||
        (frame->DLC == 1))
    {
        if (frame->DLC == 7)
        {
            stConfig.stWiper.nSwipeInput = frame->data8[1];
            stConfig.stWiper.nSpeedInput = frame->data8[2];
            stConfig.stWiper.eSpeedMap[0] = static_cast<WiperSpeed>((frame->data8[3] & 0x0F));
            stConfig.stWiper.eSpeedMap[1] = static_cast<WiperSpeed>((frame->data8[3] & 0xF0) >> 4);
            stConfig.stWiper.eSpeedMap[2] = static_cast<WiperSpeed>((frame->data8[4] & 0x0F));
            stConfig.stWiper.eSpeedMap[3] = static_cast<WiperSpeed>((frame->data8[4] & 0xF0) >> 4);
            stConfig.stWiper.eSpeedMap[4] = static_cast<WiperSpeed>((frame->data8[5] & 0x0F));
            stConfig.stWiper.eSpeedMap[5] = static_cast<WiperSpeed>((frame->data8[5] & 0xF0) >> 4);
            stConfig.stWiper.eSpeedMap[6] = static_cast<WiperSpeed>((frame->data8[6] & 0x0F));
            stConfig.stWiper.eSpeedMap[7] = static_cast<WiperSpeed>((frame->data8[6] & 0xF0) >> 4);
        }

        CANTxFrame tx;
        tx.DLC = 7;
        tx.IDE = CAN_IDE_STD;

        tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::WiperSpeed);
        tx.data8[1] = stConfig.stWiper.nSwipeInput;
        tx.data8[2] = stConfig.stWiper.nSpeedInput;
        tx.data8[3] = ((static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[1]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[0]) & 0x0F);
        tx.data8[4] = ((static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[3]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[2]) & 0x0F);
        tx.data8[5] = ((static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[5]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[4]) & 0x0F);
        tx.data8[6] = ((static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[7]) & 0x0F) << 4) +
                      (static_cast<uint8_t>(stConfig.stWiper.eSpeedMap[6]) & 0x0F);
        tx.data8[7] = 0;

        tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        PostTxFrame(&tx);

        if(frame->DLC == 7)
            return MsgCmdRx::WiperSpeed;
    }

    return MsgCmdRx::Null;
}

MsgCmdRx WiperDelaysMsg(CANRxFrame *frame)
{
    // DLC 7 = Set wiper delay settings
    // DLC 1 = Get wiper delay settings

    if ((frame->DLC == 7) ||
        (frame->DLC == 1))
    {
        if (frame->DLC == 7)
        {
            stConfig.stWiper.nIntermitTime[0] = frame->data8[1] * 100;
            stConfig.stWiper.nIntermitTime[1] = frame->data8[2] * 100;
            stConfig.stWiper.nIntermitTime[2] = frame->data8[3] * 100;
            stConfig.stWiper.nIntermitTime[3] = frame->data8[4] * 100;
            stConfig.stWiper.nIntermitTime[4] = frame->data8[5] * 100;
            stConfig.stWiper.nIntermitTime[5] = frame->data8[6] * 100;
        }

        CANTxFrame tx;
        tx.DLC = 7;
        tx.IDE = CAN_IDE_STD;

        tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::WiperDelays);
        tx.data8[1] = stConfig.stWiper.nIntermitTime[0] / 100;
        tx.data8[2] = stConfig.stWiper.nIntermitTime[1] / 100;
        tx.data8[3] = stConfig.stWiper.nIntermitTime[2] / 100;
        tx.data8[4] = stConfig.stWiper.nIntermitTime[3] / 100;
        tx.data8[5] = stConfig.stWiper.nIntermitTime[4] / 100;
        tx.data8[6] = stConfig.stWiper.nIntermitTime[5] / 100;
        tx.data8[7] = 0;

        tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        PostTxFrame(&tx);

        if(frame->DLC == 7)
            return MsgCmdRx::WiperDelays;
    }

    return MsgCmdRx::Null;
}

MsgCmdRx FlasherMsg(CANRxFrame *frame)
{
    // DLC 6 = Set flasher settings
    // DLC 2 = Get flasher settings

    if ((frame->DLC == 6) ||
        (frame->DLC == 2))
    {
        uint8_t nIndex = (frame->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_FLASHERS)
        {
            if (frame->DLC == 6)
            {
                stConfig.stFlasher[nIndex].bEnabled = (frame->data8[1] & 0x01);
                stConfig.stFlasher[nIndex].bSingleCycle = (frame->data8[1] & 0x02) >> 1;
                stConfig.stFlasher[nIndex].nInput = frame->data8[2];

                stConfig.stFlasher[nIndex].nFlashOnTime = frame->data8[4] * 100;
                stConfig.stFlasher[nIndex].nFlashOffTime = frame->data8[5] * 100;
            }

            CANTxFrame tx;
            tx.DLC = 6;
            tx.IDE = CAN_IDE_STD;

            tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::Flashers);
            tx.data8[1] = ((nIndex & 0x0F) << 4) +
                          ((stConfig.stFlasher[nIndex].bSingleCycle & 0x01) << 1) +
                          (stConfig.stFlasher[nIndex].bEnabled & 0x01);
            tx.data8[2] = stConfig.stFlasher[nIndex].nInput;

            tx.data8[4] = stConfig.stFlasher[nIndex].nFlashOnTime / 100;
            tx.data8[5] = stConfig.stFlasher[nIndex].nFlashOffTime / 100;
            tx.data8[6] = 0;
            tx.data8[7] = 0;

            tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
            PostTxFrame(&tx);

            if(frame->DLC == 6)
                return MsgCmdRx::Flashers;
        }
    }

    return MsgCmdRx::Null;
}

MsgCmdRx StarterMsg(CANRxFrame *frame)
{
    // DLC 4 = Set starter settings
    // DLC 1 = Get starter settings

    if ((frame->DLC == 4) ||
        (frame->DLC == 1))
    {
        if (frame->DLC == 4)
        {
            stConfig.stStarter.bEnabled = (frame->data8[1] & 0x01);
            stConfig.stStarter.nInput = frame->data8[2];
            stConfig.stStarter.bDisableOut[0] = (frame->data8[3] & 0x01);
            stConfig.stStarter.bDisableOut[1] = (frame->data8[3] & 0x02) >> 1;
            stConfig.stStarter.bDisableOut[2] = (frame->data8[3] & 0x04) >> 2;
            stConfig.stStarter.bDisableOut[3] = (frame->data8[3] & 0x08) >> 3;
            stConfig.stStarter.bDisableOut[4] = (frame->data8[3] & 0x10) >> 4;
            stConfig.stStarter.bDisableOut[5] = (frame->data8[3] & 0x20) >> 5;
            stConfig.stStarter.bDisableOut[6] = (frame->data8[3] & 0x40) >> 6;
            stConfig.stStarter.bDisableOut[7] = (frame->data8[3] & 0x80) >> 7;
        }

        CANTxFrame tx;
        tx.DLC = 4;
        tx.IDE = CAN_IDE_STD;

        tx.data8[0] = static_cast<uint8_t>(MsgCmdTx::Starter);
        tx.data8[1] = (stConfig.stStarter.bEnabled & 0x01);
        tx.data8[2] = stConfig.stStarter.nInput;
        tx.data8[3] = ((stConfig.stStarter.bDisableOut[7] & 0x01) << 7) +
                      ((stConfig.stStarter.bDisableOut[6] & 0x01) << 6) +
                      ((stConfig.stStarter.bDisableOut[5] & 0x01) << 5) +
                      ((stConfig.stStarter.bDisableOut[4] & 0x01) << 4) +
                      ((stConfig.stStarter.bDisableOut[3] & 0x01) << 3) +
                      ((stConfig.stStarter.bDisableOut[2] & 0x01) << 2) +
                      ((stConfig.stStarter.bDisableOut[1] & 0x01) << 1) +
                      (stConfig.stStarter.bDisableOut[0] & 0x01);
        tx.data8[4] = 0;
        tx.data8[5] = 0;
        tx.data8[6] = 0;
        tx.data8[7] = 0;

        tx.SID = stConfig.stCanOutput.nBaseId + TX_SETTINGS_ID_OFFSET;
        PostTxFrame(&tx);

        if(frame->DLC == 4)
            return MsgCmdRx::Starter;
    }

    return MsgCmdRx::Null;
}

MsgCmdRx ConfigHandler(CANRxFrame *frame)
{
    if ((frame->SID != stConfig.stCanOutput.nBaseId - 1) ||
        (frame->EID != stConfig.stCanOutput.nBaseId - 1))
    {
        return MsgCmdRx::Null;
    }

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Can))
        return CanMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Inputs))
        return InputMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Outputs))
        return OutputMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::CanInputs))
        return CanInputMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::VirtualInputs))
        return VirtualInputMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Wiper))
        return WiperMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::WiperSpeed))
        return WiperSpeedMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::WiperDelays))
        return WiperDelaysMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Flashers))
        return FlasherMsg(frame);

    if (frame->data8[0] == static_cast<uint8_t>(MsgCmdRx::Starter))
        return StarterMsg(frame);

    return MsgCmdRx::Null;
}