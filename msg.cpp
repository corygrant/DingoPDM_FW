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

CANTxFrame TxMsg0()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 0 (Digital inputs 1-2) and Device Status
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 0;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = (GetInputVal(1) << 1) + GetInputVal(0);
    stMsg.data8[1] = static_cast<uint8_t>(GetPdmState()) + (PDM_TYPE << 4);
    stMsg.data16[1] = (uint16_t)GetTotalCurrent();
    stMsg.data16[2] = (uint16_t)(GetBattVolt() * 10);
    stMsg.data16[3] = (uint16_t)GetBoardTemp();

    return stMsg;
}

CANTxFrame TxMsg1()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 1 (Out 1-4 Current)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 1;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data16[0] = GetOutputCurrent(0);
    stMsg.data16[1] = GetOutputCurrent(1);
    stMsg.data16[2] = GetOutputCurrent(2);
    stMsg.data16[3] = GetOutputCurrent(3);

    return stMsg;
}

CANTxFrame TxMsg2()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 2 (Out 5-8 Current)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 2;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data16[0] = GetOutputCurrent(4);
    stMsg.data16[1] = GetOutputCurrent(5);
    stMsg.data16[2] = GetOutputCurrent(6);
    stMsg.data16[3] = GetOutputCurrent(7);

    return stMsg;
}

CANTxFrame TxMsg3()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 3 (Out 1-8 Status)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 3;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = (static_cast<uint8_t>(GetOutputState(1)) << 4) + static_cast<uint8_t>(GetOutputState(0));
    stMsg.data8[1] = (static_cast<uint8_t>(GetOutputState(3)) << 4) + static_cast<uint8_t>(GetOutputState(2));
    stMsg.data8[2] = (static_cast<uint8_t>(GetOutputState(5)) << 4) + static_cast<uint8_t>(GetOutputState(4));
    stMsg.data8[3] = (static_cast<uint8_t>(GetOutputState(7)) << 4) + static_cast<uint8_t>(GetOutputState(6));
    stMsg.data8[4] = (GetWiperFastOut() << 1) + GetWiperSlowOut();
    stMsg.data8[5] = (static_cast<uint8_t>(GetWiperState()) << 4) + static_cast<uint8_t>(GetWiperSpeed());
    stMsg.data8[6] = (GetFlasherVal(3) << 3) + (GetFlasherVal(2) << 2) +
                     (GetFlasherVal(1) << 1) + (GetFlasherVal(0));
    stMsg.data8[7] = 0;

    return stMsg;
}

CANTxFrame TxMsg4()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 4 (Out 1-8 Reset Count)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 4;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = GetOutputOcCount(0);
    stMsg.data8[1] = GetOutputOcCount(1);
    stMsg.data8[2] = GetOutputOcCount(2);
    stMsg.data8[3] = GetOutputOcCount(3);
    stMsg.data8[4] = GetOutputOcCount(4);
    stMsg.data8[5] = GetOutputOcCount(5);
    stMsg.data8[6] = GetOutputOcCount(6);
    stMsg.data8[7] = GetOutputOcCount(7);

    return stMsg;
}

CANTxFrame TxMsg5()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 5 (CAN Inputs and Virtual Inputs)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 5;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = ((GetCanInOutput(7) & 0x01) << 7) + ((GetCanInOutput(6) & 0x01) << 6) + ((GetCanInOutput(5) & 0x01) << 5) +
                     ((GetCanInOutput(4) & 0x01) << 4) + ((GetCanInOutput(3) & 0x01) << 3) + ((GetCanInOutput(2) & 0x01) << 2) +
                     ((GetCanInOutput(1) & 0x01) << 1) + (GetCanInOutput(0) & 0x01);
    stMsg.data8[1] = ((GetCanInOutput(15) & 0x01) << 7) + ((GetCanInOutput(14) & 0x01) << 6) + ((GetCanInOutput(13) & 0x01) << 5) +
                     ((GetCanInOutput(12) & 0x01) << 4) + ((GetCanInOutput(11) & 0x01) << 3) + ((GetCanInOutput(10) & 0x01) << 2) +
                     ((GetCanInOutput(9) & 0x01) << 1) + (GetCanInOutput(8) & 0x01);
    stMsg.data8[2] = ((GetCanInOutput(23) & 0x01) << 7) + ((GetCanInOutput(22) & 0x01) << 6) + ((GetCanInOutput(21) & 0x01) << 5) +
                     ((GetCanInOutput(20) & 0x01) << 4) + ((GetCanInOutput(19) & 0x01) << 3) + ((GetCanInOutput(18) & 0x01) << 2) +
                     ((GetCanInOutput(17) & 0x01) << 1) + (GetCanInOutput(16) & 0x01);
    stMsg.data8[3] = ((GetCanInOutput(31) & 0x01) << 7) + ((GetCanInOutput(30) & 0x01) << 6) + ((GetCanInOutput(29) & 0x01) << 5) +
                     ((GetCanInOutput(28) & 0x01) << 4) + ((GetCanInOutput(27) & 0x01) << 3) + ((GetCanInOutput(26) & 0x01) << 2) +
                     ((GetCanInOutput(25) & 0x01) << 1) + (GetCanInOutput(24) & 0x01);
    stMsg.data8[4] = ((GetVirtInVal(7) & 0x01) << 7) + ((GetVirtInVal(6) & 0x01) << 6) + ((GetVirtInVal(5) & 0x01) << 5) +
                     ((GetVirtInVal(4) & 0x01) << 4) + ((GetVirtInVal(3) & 0x01) << 3) + ((GetVirtInVal(2) & 0x01) << 2) +
                     ((GetVirtInVal(1) & 0x01) << 1) + (GetVirtInVal(0) & 0x01);
    stMsg.data8[5] = ((GetVirtInVal(15) & 0x01) << 7) + ((GetVirtInVal(14) & 0x01) << 6) + ((GetVirtInVal(13) & 0x01) << 5) +
                     ((GetVirtInVal(12) & 0x01) << 4) + ((GetVirtInVal(11) & 0x01) << 3) + ((GetVirtInVal(10) & 0x01) << 2) +
                     ((GetVirtInVal(9) & 0x01) << 1) + (GetVirtInVal(8) & 0x01);
     stMsg.data8[6] = 0;
     stMsg.data8[7] = 0;

    return stMsg;
}

CANTxFrame TxMsg6()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 6 (Counters and Conditions)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 6;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = GetCounterVal(0);
    stMsg.data8[1] = GetCounterVal(1);
    stMsg.data8[2] = GetCounterVal(2);
    stMsg.data8[3] = GetCounterVal(3);
    stMsg.data8[4] = ((GetConditionVal(7) & 0x01) << 7) + ((GetConditionVal(6) & 0x01) << 6) + ((GetConditionVal(5) & 0x01) << 5) +
                     ((GetConditionVal(4) & 0x01) << 4) + ((GetConditionVal(3) & 0x01) << 3) + ((GetConditionVal(2) & 0x01) << 2) +
                     ((GetConditionVal(1) & 0x01) << 1) + (GetConditionVal(0) & 0x01);
    stMsg.data8[5] = ((GetConditionVal(15) & 0x01) << 7) + ((GetConditionVal(14) & 0x01) << 6) + ((GetConditionVal(13) & 0x01) << 5) +
                     ((GetConditionVal(12) & 0x01) << 4) + ((GetConditionVal(11) & 0x01) << 3) + ((GetConditionVal(10) & 0x01) << 2) +
                     ((GetConditionVal(9) & 0x01) << 1) + (GetConditionVal(8) & 0x01);
    stMsg.data8[6] = ((GetConditionVal(23) & 0x01) << 7) + ((GetConditionVal(22) & 0x01) << 6) + ((GetConditionVal(21) & 0x01) << 5) +
                     ((GetConditionVal(20) & 0x01) << 4) + ((GetConditionVal(19) & 0x01) << 3) + ((GetConditionVal(18) & 0x01) << 2) +
                     ((GetConditionVal(17) & 0x01) << 1) + (GetConditionVal(16) & 0x01);
    stMsg.data8[7] = ((GetConditionVal(31) & 0x01) << 7) + ((GetConditionVal(30) & 0x01) << 6) + ((GetConditionVal(29) & 0x01) << 5) +
                     ((GetConditionVal(28) & 0x01) << 4) + ((GetConditionVal(27) & 0x01) << 3) + ((GetConditionVal(26) & 0x01) << 2) +
                     ((GetConditionVal(25) & 0x01) << 1) + (GetConditionVal(24) & 0x01);

    return stMsg;
}

CANTxFrame TxMsg7()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 7 (CAN Input Values 1-4)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 7;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data16[0] = GetCanInVal(0);
    stMsg.data16[1] = GetCanInVal(1);
    stMsg.data16[2] = GetCanInVal(2);
    stMsg.data16[3] = GetCanInVal(3); 

    return stMsg;
}

CANTxFrame TxMsg8()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 8 (CAN Input Values 5-8)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 8;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data16[0] = GetCanInVal(4);
    stMsg.data16[1] = GetCanInVal(5);
    stMsg.data16[2] = GetCanInVal(6);
    stMsg.data16[3] = GetCanInVal(7); 

    return stMsg;
}

CANTxFrame TxMsg9()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 9 (CAN Input Values 9-12)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 9;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data16[0] = GetCanInVal(8);
    stMsg.data16[1] = GetCanInVal(9);
    stMsg.data16[2] = GetCanInVal(10);
    stMsg.data16[3] = GetCanInVal(11); 

    return stMsg;
}

CANTxFrame TxMsg10()
{
    CANTxFrame stMsg;
    stMsg.SID = stConfig.stCanOutput.nBaseId + 10;
    stMsg.DLC = 8;
    stMsg.data16[0] = GetCanInVal(12);
    stMsg.data16[1] = GetCanInVal(13);
    stMsg.data16[2] = GetCanInVal(14);
    stMsg.data16[3] = GetCanInVal(15);
    return stMsg;
}

CANTxFrame TxMsg11()
{
    CANTxFrame stMsg;
    stMsg.SID = stConfig.stCanOutput.nBaseId + 11;
    stMsg.DLC = 8;
    stMsg.data16[0] = GetCanInVal(16);
    stMsg.data16[1] = GetCanInVal(17);
    stMsg.data16[2] = GetCanInVal(18);
    stMsg.data16[3] = GetCanInVal(19);
    return stMsg;
}

CANTxFrame TxMsg12()
{
    CANTxFrame stMsg;
    stMsg.SID = stConfig.stCanOutput.nBaseId + 12;
    stMsg.DLC = 8;
    stMsg.data16[0] = GetCanInVal(20);
    stMsg.data16[1] = GetCanInVal(21);
    stMsg.data16[2] = GetCanInVal(22);
    stMsg.data16[3] = GetCanInVal(23);
    return stMsg;
}

CANTxFrame TxMsg13()
{
    CANTxFrame stMsg;
    stMsg.SID = stConfig.stCanOutput.nBaseId + 13;
    stMsg.DLC = 8;
    stMsg.data16[0] = GetCanInVal(24);
    stMsg.data16[1] = GetCanInVal(25);
    stMsg.data16[2] = GetCanInVal(26);
    stMsg.data16[3] = GetCanInVal(27);
    return stMsg;
}

CANTxFrame TxMsg14()
{
    CANTxFrame stMsg;
    stMsg.SID = stConfig.stCanOutput.nBaseId + 14;
    stMsg.DLC = 8;
    stMsg.data16[0] = GetCanInVal(28);
    stMsg.data16[1] = GetCanInVal(29);
    stMsg.data16[2] = GetCanInVal(30);
    stMsg.data16[3] = GetCanInVal(31);
    return stMsg;
}

CANTxFrame TxMsg15()
{
    CANTxFrame stMsg;
    stMsg.SID = stConfig.stCanOutput.nBaseId + 15;
    stMsg.DLC = 8;
    stMsg.data8[0] = GetOutputDC(0);
    stMsg.data8[1] = GetOutputDC(1);
    stMsg.data8[2] = GetOutputDC(2);
    stMsg.data8[3] = GetOutputDC(3);
    stMsg.data8[4] = GetOutputDC(4);
    stMsg.data8[5] = GetOutputDC(5);
    stMsg.data8[6] = GetOutputDC(6);
    stMsg.data8[7] = GetOutputDC(7); 
    return stMsg;
}