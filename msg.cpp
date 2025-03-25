#include "msg.h"
#include "config.h"
#include "profet.h"
#include "digital.h"
#include "pdm.h"

void InfoMsg::Check(bool bTrigger, uint16_t nId, uint16_t nData0, uint16_t nData1, uint16_t nData2)
{
    if (!bTrigger)
    {
        bSent = false;
        return;
    }

    if (bSent)
        return;

    CANTxFrame tx;
    tx.DLC = 8;

    tx.data8[0] = static_cast<uint8_t>(m_type);
    tx.data8[1] = static_cast<uint8_t>(m_src);
    tx.data16[1] = nData0;
    tx.data16[2] = nData1;
    tx.data16[3] = nData2;

    tx.SID = nId + TX_MSG_ID_OFFSET;
    tx.IDE = CAN_IDE_STD;
    PostTxFrame(&tx);

    bSent = true;
}

CANTxMsg TxMsg0()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 0 (Digital inputs 1-2) and Device Status
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 0;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data8[0] = (GetInputVal(1) << 1) + GetInputVal(0);
    stMsg.frame.data8[1] = static_cast<uint8_t>(GetPdmState()) + (PDM_TYPE << 4);
    stMsg.frame.data16[1] = (uint16_t)GetTotalCurrent(); //Already scaled by 10
    stMsg.frame.data16[2] = (uint16_t)(GetBattVolt() * 10.0);
    stMsg.frame.data16[3] = (uint16_t)(GetBoardTemp() * 10.0);

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg1()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 1 (Out 1-4 Current)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 1;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data16[0] = GetOutputCurrent(0);
    stMsg.frame.data16[1] = GetOutputCurrent(1);
    stMsg.frame.data16[2] = GetOutputCurrent(2);
    stMsg.frame.data16[3] = GetOutputCurrent(3);

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg2()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 2 (Out 5-8 Current)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 2;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data16[0] = GetOutputCurrent(4);
    stMsg.frame.data16[1] = GetOutputCurrent(5);
    stMsg.frame.data16[2] = GetOutputCurrent(6);
    stMsg.frame.data16[3] = GetOutputCurrent(7);

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg3()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 3 (Out 1-8 Status)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 3;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data8[0] = (static_cast<uint8_t>(GetOutputState(1)) << 4) + static_cast<uint8_t>(GetOutputState(0));
    stMsg.frame.data8[1] = (static_cast<uint8_t>(GetOutputState(3)) << 4) + static_cast<uint8_t>(GetOutputState(2));
    stMsg.frame.data8[2] = (static_cast<uint8_t>(GetOutputState(5)) << 4) + static_cast<uint8_t>(GetOutputState(4));
    stMsg.frame.data8[3] = (static_cast<uint8_t>(GetOutputState(7)) << 4) + static_cast<uint8_t>(GetOutputState(6));
    stMsg.frame.data8[4] = (GetWiperFastOut() << 1) + GetWiperSlowOut();
    stMsg.frame.data8[5] = (static_cast<uint8_t>(GetWiperState()) << 4) + static_cast<uint8_t>(GetWiperSpeed());
    stMsg.frame.data8[6] = (GetFlasherVal(3) << 3) + (GetFlasherVal(2) << 2) +
                           (GetFlasherVal(1) << 1) + (GetFlasherVal(0));
    stMsg.frame.data8[7] = 0;

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg4()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 4 (Out 1-8 Reset Count)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 4;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data8[0] = GetOutputOcCount(0);
    stMsg.frame.data8[1] = GetOutputOcCount(1);
    stMsg.frame.data8[2] = GetOutputOcCount(2);
    stMsg.frame.data8[3] = GetOutputOcCount(3);
    stMsg.frame.data8[4] = GetOutputOcCount(4);
    stMsg.frame.data8[5] = GetOutputOcCount(5);
    stMsg.frame.data8[6] = GetOutputOcCount(6);
    stMsg.frame.data8[7] = GetOutputOcCount(7);

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg5()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 5 (CAN Inputs and Virtual Inputs)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 5;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data8[0] = ((GetCanInOutput(7) & 0x01) << 7) + ((GetCanInOutput(6) & 0x01) << 6) + ((GetCanInOutput(5) & 0x01) << 5) +
                           ((GetCanInOutput(4) & 0x01) << 4) + ((GetCanInOutput(3) & 0x01) << 3) + ((GetCanInOutput(2) & 0x01) << 2) +
                           ((GetCanInOutput(1) & 0x01) << 1) + (GetCanInOutput(0) & 0x01);
    stMsg.frame.data8[1] = ((GetCanInOutput(15) & 0x01) << 7) + ((GetCanInOutput(14) & 0x01) << 6) + ((GetCanInOutput(13) & 0x01) << 5) +
                           ((GetCanInOutput(12) & 0x01) << 4) + ((GetCanInOutput(11) & 0x01) << 3) + ((GetCanInOutput(10) & 0x01) << 2) +
                           ((GetCanInOutput(9) & 0x01) << 1) + (GetCanInOutput(8) & 0x01);
    stMsg.frame.data8[2] = ((GetCanInOutput(23) & 0x01) << 7) + ((GetCanInOutput(22) & 0x01) << 6) + ((GetCanInOutput(21) & 0x01) << 5) +
                           ((GetCanInOutput(20) & 0x01) << 4) + ((GetCanInOutput(19) & 0x01) << 3) + ((GetCanInOutput(18) & 0x01) << 2) +
                           ((GetCanInOutput(17) & 0x01) << 1) + (GetCanInOutput(16) & 0x01);
    stMsg.frame.data8[3] = ((GetCanInOutput(31) & 0x01) << 7) + ((GetCanInOutput(30) & 0x01) << 6) + ((GetCanInOutput(29) & 0x01) << 5) +
                           ((GetCanInOutput(28) & 0x01) << 4) + ((GetCanInOutput(27) & 0x01) << 3) + ((GetCanInOutput(26) & 0x01) << 2) +
                           ((GetCanInOutput(25) & 0x01) << 1) + (GetCanInOutput(24) & 0x01);
    stMsg.frame.data8[4] = ((GetVirtInVal(7) & 0x01) << 7) + ((GetVirtInVal(6) & 0x01) << 6) + ((GetVirtInVal(5) & 0x01) << 5) +
                           ((GetVirtInVal(4) & 0x01) << 4) + ((GetVirtInVal(3) & 0x01) << 3) + ((GetVirtInVal(2) & 0x01) << 2) +
                           ((GetVirtInVal(1) & 0x01) << 1) + (GetVirtInVal(0) & 0x01);
    stMsg.frame.data8[5] = ((GetVirtInVal(15) & 0x01) << 7) + ((GetVirtInVal(14) & 0x01) << 6) + ((GetVirtInVal(13) & 0x01) << 5) +
                           ((GetVirtInVal(12) & 0x01) << 4) + ((GetVirtInVal(11) & 0x01) << 3) + ((GetVirtInVal(10) & 0x01) << 2) +
                           ((GetVirtInVal(9) & 0x01) << 1) + (GetVirtInVal(8) & 0x01);
    stMsg.frame.data8[6] = 0;
    stMsg.frame.data8[7] = 0;

    stMsg.bSend = GetAnyCanInEnable() || GetAnyVirtInEnable();

    return stMsg;
}

CANTxMsg TxMsg6()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 6 (Counters and Conditions)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 6;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data8[0] = GetCounterVal(0);
    stMsg.frame.data8[1] = GetCounterVal(1);
    stMsg.frame.data8[2] = GetCounterVal(2);
    stMsg.frame.data8[3] = GetCounterVal(3);
    stMsg.frame.data8[4] = ((GetConditionVal(7) & 0x01) << 7) + ((GetConditionVal(6) & 0x01) << 6) + ((GetConditionVal(5) & 0x01) << 5) +
                           ((GetConditionVal(4) & 0x01) << 4) + ((GetConditionVal(3) & 0x01) << 3) + ((GetConditionVal(2) & 0x01) << 2) +
                           ((GetConditionVal(1) & 0x01) << 1) + (GetConditionVal(0) & 0x01);
    stMsg.frame.data8[5] = ((GetConditionVal(15) & 0x01) << 7) + ((GetConditionVal(14) & 0x01) << 6) + ((GetConditionVal(13) & 0x01) << 5) +
                           ((GetConditionVal(12) & 0x01) << 4) + ((GetConditionVal(11) & 0x01) << 3) + ((GetConditionVal(10) & 0x01) << 2) +
                           ((GetConditionVal(9) & 0x01) << 1) + (GetConditionVal(8) & 0x01);
    stMsg.frame.data8[6] = ((GetConditionVal(23) & 0x01) << 7) + ((GetConditionVal(22) & 0x01) << 6) + ((GetConditionVal(21) & 0x01) << 5) +
                           ((GetConditionVal(20) & 0x01) << 4) + ((GetConditionVal(19) & 0x01) << 3) + ((GetConditionVal(18) & 0x01) << 2) +
                           ((GetConditionVal(17) & 0x01) << 1) + (GetConditionVal(16) & 0x01);
    stMsg.frame.data8[7] = ((GetConditionVal(31) & 0x01) << 7) + ((GetConditionVal(30) & 0x01) << 6) + ((GetConditionVal(29) & 0x01) << 5) +
                           ((GetConditionVal(28) & 0x01) << 4) + ((GetConditionVal(27) & 0x01) << 3) + ((GetConditionVal(26) & 0x01) << 2) +
                           ((GetConditionVal(25) & 0x01) << 1) + (GetConditionVal(24) & 0x01);

    stMsg.bSend = GetAnyCounterEnable() || GetAnyConditionEnable();

    return stMsg;
}

CANTxMsg TxMsg7()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 7 (CAN Input Values 1-4)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 7;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data16[0] = GetCanInVal(0);
    stMsg.frame.data16[1] = GetCanInVal(1);
    stMsg.frame.data16[2] = GetCanInVal(2);
    stMsg.frame.data16[3] = GetCanInVal(3);

    stMsg.bSend = GetCanInEnable(0) || GetCanInEnable(1) || GetCanInEnable(2) || GetCanInEnable(3);

    return stMsg;
}

CANTxMsg TxMsg8()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 8 (CAN Input Values 5-8)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 8;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data16[0] = GetCanInVal(4);
    stMsg.frame.data16[1] = GetCanInVal(5);
    stMsg.frame.data16[2] = GetCanInVal(6);
    stMsg.frame.data16[3] = GetCanInVal(7);

    stMsg.bSend = GetCanInEnable(4) || GetCanInEnable(5) || GetCanInEnable(6) || GetCanInEnable(7);

    return stMsg;
}

CANTxMsg TxMsg9()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 9 (CAN Input Values 9-12)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 9;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data16[0] = GetCanInVal(8);
    stMsg.frame.data16[1] = GetCanInVal(9);
    stMsg.frame.data16[2] = GetCanInVal(10);
    stMsg.frame.data16[3] = GetCanInVal(11);

    stMsg.bSend = GetCanInEnable(8) || GetCanInEnable(9) || GetCanInEnable(10) || GetCanInEnable(11);

    return stMsg;
}

CANTxMsg TxMsg10()
{
    CANTxMsg stMsg;
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 10;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetCanInVal(12);
    stMsg.frame.data16[1] = GetCanInVal(13);
    stMsg.frame.data16[2] = GetCanInVal(14);
    stMsg.frame.data16[3] = GetCanInVal(15);

    stMsg.bSend = GetCanInEnable(12) || GetCanInEnable(13) || GetCanInEnable(14) || GetCanInEnable(15);

    return stMsg;
}

CANTxMsg TxMsg11()
{
    CANTxMsg stMsg;
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 11;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetCanInVal(16);
    stMsg.frame.data16[1] = GetCanInVal(17);
    stMsg.frame.data16[2] = GetCanInVal(18);
    stMsg.frame.data16[3] = GetCanInVal(19);

    stMsg.bSend = GetCanInEnable(16) || GetCanInEnable(17) || GetCanInEnable(18) || GetCanInEnable(19);

    return stMsg;
}

CANTxMsg TxMsg12()
{
    CANTxMsg stMsg;
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 12;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetCanInVal(20);
    stMsg.frame.data16[1] = GetCanInVal(21);
    stMsg.frame.data16[2] = GetCanInVal(22);
    stMsg.frame.data16[3] = GetCanInVal(23);

    stMsg.bSend = GetCanInEnable(20) || GetCanInEnable(21) || GetCanInEnable(22) || GetCanInEnable(23);

    return stMsg;
}

CANTxMsg TxMsg13()
{
    CANTxMsg stMsg;
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 13;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetCanInVal(24);
    stMsg.frame.data16[1] = GetCanInVal(25);
    stMsg.frame.data16[2] = GetCanInVal(26);
    stMsg.frame.data16[3] = GetCanInVal(27);

    stMsg.bSend = GetCanInEnable(24) || GetCanInEnable(25) || GetCanInEnable(26) || GetCanInEnable(27);

    return stMsg;
}

CANTxMsg TxMsg14()
{
    CANTxMsg stMsg;
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 14;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetCanInVal(28);
    stMsg.frame.data16[1] = GetCanInVal(29);
    stMsg.frame.data16[2] = GetCanInVal(30);
    stMsg.frame.data16[3] = GetCanInVal(31);

    stMsg.bSend = GetCanInEnable(28) || GetCanInEnable(29) || GetCanInEnable(30) || GetCanInEnable(31);

    return stMsg;
}

CANTxMsg TxMsg15()
{
    CANTxMsg stMsg;
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 15;
    stMsg.frame.DLC = 8;
    stMsg.frame.data8[0] = GetOutputDC(0);
    stMsg.frame.data8[1] = GetOutputDC(1);
    stMsg.frame.data8[2] = GetOutputDC(2);
    stMsg.frame.data8[3] = GetOutputDC(3);
    stMsg.frame.data8[4] = GetOutputDC(4);
    stMsg.frame.data8[5] = GetOutputDC(5);
    stMsg.frame.data8[6] = GetOutputDC(6);
    stMsg.frame.data8[7] = GetOutputDC(7);

    stMsg.bSend = true; // Always send

    return stMsg;
}