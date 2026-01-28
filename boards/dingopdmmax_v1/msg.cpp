#include "msg.h"
#include "config.h"
#include "profet.h"
#include "digital.h"
#include "pdm.h"
#include "status.h"


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
    stMsg.frame.data64[0] = 0;

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
    stMsg.frame.data8[2] = 0;
    stMsg.frame.data8[3] = 0;
    stMsg.frame.data8[4] = (GetWiperFastOut() << 1) + GetWiperSlowOut();
    stMsg.frame.data8[5] = (static_cast<uint8_t>(GetWiperState()) << 4) + static_cast<uint8_t>(GetWiperSpeed());
    stMsg.frame.data8[6] = (GetFlasherVal(3) << 3) + (GetFlasherVal(2) << 2) +
                           (GetFlasherVal(1) << 1) + GetFlasherVal(0);
    stMsg.frame.data8[7] = 0;

    stMsg.bSend = true; // Always send

    return stMsg;
}

CANTxMsg TxMsg4()
{
    CANTxMsg stMsg;
    //=======================================================
    // Build Msg 4 (Out 1-4 Reset Count)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 4;
    stMsg.frame.DLC = 8; // Bytes to send
    stMsg.frame.data8[0] = GetOutputOcCount(0);
    stMsg.frame.data8[1] = GetOutputOcCount(1);
    stMsg.frame.data8[2] = GetOutputOcCount(2);
    stMsg.frame.data8[3] = GetOutputOcCount(3);
    stMsg.frame.data8[4] = 0;
    stMsg.frame.data8[5] = 0;
    stMsg.frame.data8[6] = 0;
    stMsg.frame.data8[7] = 0;

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
    stMsg.frame.data8[0] = GetCounterVal(0);
    stMsg.frame.data8[1] = GetCounterVal(1);
    stMsg.frame.data8[2] = GetCounterVal(2);
    stMsg.frame.data8[3] = GetCounterVal(3);
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
    //=======================================================
    // Build Msg 10 (CAN Input Values 13-16)
    //=======================================================
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
    //=======================================================
    // Build Msg 11 (CAN Input Values 17-20)
    //=======================================================
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
    //=======================================================
    // Build Msg 12 (CAN Input Values 21-24)
    //=======================================================
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
    //=======================================================
    // Build Msg 13 (CAN Input Values 25-28)
    //=======================================================
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
    //=======================================================
    // Build Msg 14 (CAN Input Values 29-32)
    //=======================================================
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
    //=======================================================
    // Build Msg 15 (Output Duty Cycle)
    //=======================================================
    stMsg.frame.SID = stConfig.stCanOutput.nBaseId + 15;
    stMsg.frame.DLC = 8;
    stMsg.frame.data8[0] = GetOutputDC(0);
    stMsg.frame.data8[1] = GetOutputDC(1);
    stMsg.frame.data8[2] = GetOutputDC(2);
    stMsg.frame.data8[3] = GetOutputDC(3);
    stMsg.frame.data8[4] = 0;
    stMsg.frame.data8[5] = 0;
    stMsg.frame.data8[6] = 0;
    stMsg.frame.data8[7] = 0;

    stMsg.bSend = GetAnyPwmEnable();

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
    stMsg.frame.data8[0] =  GetKeypadDialVal(0,0) & 0xFF;
    stMsg.frame.data8[1] = (GetKeypadDialVal(0,0) >> 8) & 0xFF;
    stMsg.frame.data8[2] = (GetKeypadDialVal(0,1) & 0xFF);
    stMsg.frame.data8[3] = (GetKeypadDialVal(0,1) >> 8) & 0xFF;
    stMsg.frame.data8[4] =  GetKeypadDialVal(0,2) & 0xFF;
    stMsg.frame.data8[5] = (GetKeypadDialVal(0,2) >> 8) & 0xFF;
    stMsg.frame.data8[6] =  GetKeypadDialVal(0,3) & 0xFF;
    stMsg.frame.data8[7] = (GetKeypadDialVal(0,3) >> 8) & 0xFF;

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
    stMsg.frame.data8[0] =  GetKeypadDialVal(1,0) & 0xFF;
    stMsg.frame.data8[1] = (GetKeypadDialVal(1,0) >> 8) & 0xFF;
    stMsg.frame.data8[2] = (GetKeypadDialVal(1,1) & 0xFF);
    stMsg.frame.data8[3] = (GetKeypadDialVal(1,1) >> 8) & 0xFF;
    stMsg.frame.data8[4] =  GetKeypadDialVal(1,2) & 0xFF;
    stMsg.frame.data8[5] = (GetKeypadDialVal(1,2) >> 8) & 0xFF;
    stMsg.frame.data8[6] =  GetKeypadDialVal(1,3) & 0xFF;
    stMsg.frame.data8[7] = (GetKeypadDialVal(1,3) >> 8) & 0xFF;

    stMsg.bSend = GetKeypadEnable(1);

    return stMsg;
}