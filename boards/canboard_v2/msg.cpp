#include "msg.h"
#include "config.h"
#include "status.h"
#include "device.h"

uint8_t nHeartbeat = 0;

CANTxMsg TxMsg0()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 0 (Analog inputs 1-4 millivolts)
    //=======================================================
    stMsg.frame.IDE = CAN_IDE_STD;
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 0;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = (uint16_t)(GetAnalogInputMv(0));
    stMsg.frame.data16[1] = (uint16_t)(GetAnalogInputMv(1));
    stMsg.frame.data16[2] = (uint16_t)(GetAnalogInputMv(2));
    stMsg.frame.data16[3] = (uint16_t)(GetAnalogInputMv(3));

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg1()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 1 (Analog input 5 millivolts and temperature)
    //=======================================================
    stMsg.frame.IDE = CAN_IDE_STD;
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 1;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = (uint16_t)(GetAnalogInputMv(4));
    stMsg.frame.data16[1] = 0;
    stMsg.frame.data16[2] = 0;
    stMsg.frame.data16[3] = GetTemperature();

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg2()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 2 (Rotary switches, dig inputs, analog input switches, low side output status, heartbeat)
    //=======================================================
    stMsg.frame.IDE = CAN_IDE_STD;
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 2;
    stMsg.frame.DLC = 8;
    stMsg.frame.data8[0] = (GetRotarySwitchPos(1) << 4) + GetRotarySwitchPos(0);
    stMsg.frame.data8[1] = (GetRotarySwitchPos(3) << 4) + GetRotarySwitchPos(2);
    stMsg.frame.data8[2] = GetRotarySwitchPos(4);
    stMsg.frame.data8[3] = (GetFlasherVal(3) << 3) + (GetFlasherVal(2) << 2) +
                           (GetFlasherVal(1) << 1) + GetFlasherVal(0);
    stMsg.frame.data8[4] = (GetDigInputVal(7) << 7) + (GetDigInputVal(6) << 6) + (GetDigInputVal(5) << 5) + (GetDigInputVal(4) << 4) + 
                           (GetDigInputVal(3) << 3) + (GetDigInputVal(2) << 2) + (GetDigInputVal(1) << 1) + GetDigInputVal(0);
    stMsg.frame.data8[5] = (GetAnalogSwitchVal(4) << 4) + (GetAnalogSwitchVal(3) << 3) + (GetAnalogSwitchVal(2) << 2) + 
                           (GetAnalogSwitchVal(1) << 1) + GetAnalogSwitchVal(0);
    stMsg.frame.data8[6] = (GetDigOutputState(3) << 3) + (GetDigOutputState(2) << 2) + (GetDigOutputState(1) << 1) + GetDigOutputState(0);
    stMsg.frame.data8[7] = nHeartbeat;

    nHeartbeat++; // Increment heartbeat for each msg sent

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg3()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 3 (CAN Inputs and Virtual Inputs)
    //=======================================================
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 3;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data32[0] = GetCanInOutputs();
    stMsg.frame.data32[1] = GetVirtIns();

    stMsg.bSend = GetAnyCanInEnable() || GetAnyVirtInEnable();

    return stMsg;
}

CANTxMsg TxMsg4()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 4 (Counters and Conditions)
    //=======================================================
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 4;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data8[0] = (uint8_t)GetCounterVal(0);
    stMsg.frame.data8[1] = (uint8_t)GetCounterVal(1);
    stMsg.frame.data8[2] = (uint8_t)GetCounterVal(2);
    stMsg.frame.data8[3] = (uint8_t)GetCounterVal(3);
    stMsg.frame.data32[1] = GetConditions();

    stMsg.bSend = GetAnyCounterEnable() || GetAnyConditionEnable();

    return stMsg;
}

CANTxMsg TxMsg5()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 5 (CAN Input Values 1-2)
    //=======================================================
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 5;
    stMsg.frame.DLC = 8; // Bytes to send
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(0), 0, 32, GetCanInFactor(0), GetCanInOffset(0), GetCanInByteOrder(0));
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(1), 33, 32, GetCanInFactor(1), GetCanInOffset(1), GetCanInByteOrder(1));

    stMsg.bSend = GetCanInEnable(0) || GetCanInEnable(1);

    return stMsg;
}

CANTxMsg TxMsg6()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 6 (CAN Input Values 3-4)
    //=======================================================
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 6;
    stMsg.frame.DLC = 8; // Bytes to send
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(2), 0, 32, GetCanInFactor(2), GetCanInOffset(2), GetCanInByteOrder(2));
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(3), 33, 32, GetCanInFactor(3), GetCanInOffset(3), GetCanInByteOrder(3));

    stMsg.bSend = GetCanInEnable(2) || GetCanInEnable(3);

    return stMsg;
}

CANTxMsg TxMsg7()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 7 (CAN Input Values 5-6)
    //=======================================================
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 7;
    stMsg.frame.DLC = 8; // Bytes to send
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(4), 0, 32, GetCanInFactor(4), GetCanInOffset(4), GetCanInByteOrder(4));
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(5), 33, 32, GetCanInFactor(5), GetCanInOffset(5), GetCanInByteOrder(5));

    stMsg.bSend = GetCanInEnable(4) || GetCanInEnable(5);

    return stMsg;
}

CANTxMsg TxMsg8()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 8 (CAN Input Values 7-8)
    //=======================================================
    stMsg.frame.SID = stConfig.stDevice.nBaseId + CYCLIC_TX_OFFSET + 8;
    stMsg.frame.DLC = 8; // Bytes to send
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(6), 0, 32, GetCanInFactor(6), GetCanInOffset(6), GetCanInByteOrder(6));
    Dbc::EncodeFloat(stMsg.frame.data8, GetCanInVal(7), 33, 32, GetCanInFactor(7), GetCanInOffset(7), GetCanInByteOrder(7));

    stMsg.bSend = GetCanInEnable(6) || GetCanInEnable(7);

    return stMsg;
}

