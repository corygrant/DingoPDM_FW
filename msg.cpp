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
    stMsg.frame.data8[0] = ((GetVarMap(VarMap::DigitalIn2) & 0x01) << 1) + (GetVarMap(VarMap::DigitalIn1) & 0x01);
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
    stMsg.frame.data8[4] = ((GetVarMap(VarMap::WiperFast) & 0x01) << 1) + (GetVarMap(VarMap::WiperSlow) & 0x01);
    stMsg.frame.data8[5] = (static_cast<uint8_t>(GetWiperState()) << 4) + static_cast<uint8_t>(GetWiperSpeed());
    stMsg.frame.data8[6] = ((GetVarMap(VarMap::Flasher4) & 0x01) << 3) + ((GetVarMap(VarMap::Flasher3) & 0x01) << 2) +
                           ((GetVarMap(VarMap::Flasher2) & 0x01) << 1) + (GetVarMap(VarMap::Flasher1) & 0x01);
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
    stMsg.frame.data32[0] = GetCanInOutputs();
    stMsg.frame.data32[1] = GetVirtIns();

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
    stMsg.frame.data8[0] = GetVarMap(VarMap::CounterVal1);
    stMsg.frame.data8[1] = GetVarMap(VarMap::CounterVal2);
    stMsg.frame.data8[2] = GetVarMap(VarMap::CounterVal3);
    stMsg.frame.data8[3] = GetVarMap(VarMap::CounterVal4);
    stMsg.frame.data32[1] = GetConditions();

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
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal1);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal2);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal3);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal4);

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
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal5);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal6);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal7);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal8);

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
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal9);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal10);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal11);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal12);

    stMsg.bSend = GetCanInEnable(8) || GetCanInEnable(9) || GetCanInEnable(10) || GetCanInEnable(11);

    return stMsg;
}

CANTxMsg TxMsg10()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 10 (CAN Input Values 13-16)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 10;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal13);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal14);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal15);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal16);

    stMsg.bSend = GetCanInEnable(12) || GetCanInEnable(13) || GetCanInEnable(14) || GetCanInEnable(15);

    return stMsg;
}

CANTxMsg TxMsg11()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 11 (CAN Input Values 17-20)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 11;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal17);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal18);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal19);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal20);

    stMsg.bSend = GetCanInEnable(16) || GetCanInEnable(17) || GetCanInEnable(18) || GetCanInEnable(19);

    return stMsg;
}

CANTxMsg TxMsg12()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 12 (CAN Input Values 21-24)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 12;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal21);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal22);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal23);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal24);

    stMsg.bSend = GetCanInEnable(20) || GetCanInEnable(21) || GetCanInEnable(22) || GetCanInEnable(23);

    return stMsg;
}

CANTxMsg TxMsg13()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 13 (CAN Input Values 25-28)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 13;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal25);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal26);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal27);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal28);

    stMsg.bSend = GetCanInEnable(24) || GetCanInEnable(25) || GetCanInEnable(26) || GetCanInEnable(27);

    return stMsg;
}

CANTxMsg TxMsg14()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 14 (CAN Input Values 29-32)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 14;
    stMsg.frame.DLC = 8;
    stMsg.frame.data16[0] = GetVarMap(VarMap::CANInVal29);
    stMsg.frame.data16[1] = GetVarMap(VarMap::CANInVal30);
    stMsg.frame.data16[2] = GetVarMap(VarMap::CANInVal31);
    stMsg.frame.data16[3] = GetVarMap(VarMap::CANInVal32);

    stMsg.bSend = GetCanInEnable(28) || GetCanInEnable(29) || GetCanInEnable(30) || GetCanInEnable(31);

    return stMsg;
}

CANTxMsg TxMsg15()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 15 (Output Duty Cycle)
    //=======================================================
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

CANTxMsg TxMsg16()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 16 (Keypad Button States)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 16;
    stMsg.frame.DLC = 8;
    stMsg.frame.data8[0] =  GetKeypadButtons(0) & 0xFF;
    stMsg.frame.data8[1] = (GetKeypadButtons(0) >> 8) & 0xFF;
    stMsg.frame.data8[2] = (GetKeypadButtons(0) >> 16) & 0xFF;
    stMsg.frame.data8[3] =  GetKeypadButtons(1) & 0xFF;
    stMsg.frame.data8[4] = (GetKeypadButtons(1) >> 8) & 0xFF;
    stMsg.frame.data8[5] = (GetKeypadButtons(1) >> 16) & 0xFF;
    stMsg.frame.data8[6] = 0;
    stMsg.frame.data8[7] = 0;

    stMsg.bSend = GetAnyKeypadEnable();

    return stMsg;
}

CANTxMsg TxMsg17()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 17 (Keypad 1 Dials)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 17;
    stMsg.frame.DLC = 8;
    stMsg.frame.data8[0] =  GetVarMap(VarMap::Keypad1Dial1) & 0xFF;
    stMsg.frame.data8[1] = (GetVarMap(VarMap::Keypad1Dial1) >> 8) & 0xFF;
    stMsg.frame.data8[2] = (GetVarMap(VarMap::Keypad1Dial2) & 0xFF);
    stMsg.frame.data8[3] = (GetVarMap(VarMap::Keypad1Dial2) >> 8) & 0xFF;
    stMsg.frame.data8[4] =  GetVarMap(VarMap::Keypad1Dial3) & 0xFF;
    stMsg.frame.data8[5] = (GetVarMap(VarMap::Keypad1Dial3) >> 8) & 0xFF;
    stMsg.frame.data8[6] =  GetVarMap(VarMap::Keypad1Dial4) & 0xFF;
    stMsg.frame.data8[7] = (GetVarMap(VarMap::Keypad1Dial4) >> 8) & 0xFF;

    stMsg.bSend = GetKeypadEnable(0);

    return stMsg;
}

CANTxMsg TxMsg18()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 18 (Keypad 2 Dials)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 18;
    stMsg.frame.DLC = 8;
    stMsg.frame.data8[0] =  GetVarMap(VarMap::Keypad2Dial1) & 0xFF;
    stMsg.frame.data8[1] = (GetVarMap(VarMap::Keypad2Dial1) >> 8) & 0xFF;
    stMsg.frame.data8[2] = (GetVarMap(VarMap::Keypad2Dial2) & 0xFF);
    stMsg.frame.data8[3] = (GetVarMap(VarMap::Keypad2Dial2) >> 8) & 0xFF;
    stMsg.frame.data8[4] =  GetVarMap(VarMap::Keypad2Dial3) & 0xFF;
    stMsg.frame.data8[5] = (GetVarMap(VarMap::Keypad2Dial3) >> 8) & 0xFF;
    stMsg.frame.data8[6] =  GetVarMap(VarMap::Keypad2Dial4) & 0xFF;
    stMsg.frame.data8[7] = (GetVarMap(VarMap::Keypad2Dial4) >> 8) & 0xFF;

    stMsg.bSend = GetKeypadEnable(1);

    return stMsg;
}