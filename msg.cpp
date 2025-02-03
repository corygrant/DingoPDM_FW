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
    stMsg.data8[0] = ((GetCanInVal(7) & 0x01) << 7) + ((GetCanInVal(6) & 0x01) << 6) + ((GetCanInVal(5) & 0x01) << 5) +
                     ((GetCanInVal(4) & 0x01) << 4) + ((GetCanInVal(3) & 0x01) << 3) + ((GetCanInVal(2) & 0x01) << 2) +
                     ((GetCanInVal(1) & 0x01) << 1) + (GetCanInVal(0) & 0x01);
    stMsg.data8[1] = ((GetCanInVal(15) & 0x01) << 7) + ((GetCanInVal(14) & 0x01) << 6) + ((GetCanInVal(13) & 0x01) << 5) +
                     ((GetCanInVal(12) & 0x01) << 4) + ((GetCanInVal(11) & 0x01) << 3) + ((GetCanInVal(10) & 0x01) << 2) +
                     ((GetCanInVal(9) & 0x01) << 1) + (GetCanInVal(8) & 0x01);
    stMsg.data8[2] = ((GetCanInVal(23) & 0x01) << 7) + ((GetCanInVal(22) & 0x01) << 6) + ((GetCanInVal(21) & 0x01) << 5) +
                     ((GetCanInVal(20) & 0x01) << 4) + ((GetCanInVal(19) & 0x01) << 3) + ((GetCanInVal(18) & 0x01) << 2) +
                     ((GetCanInVal(17) & 0x01) << 1) + (GetCanInVal(16) & 0x01);
    stMsg.data8[3] = ((GetCanInVal(31) & 0x01) << 7) + ((GetCanInVal(30) & 0x01) << 6) + ((GetCanInVal(29) & 0x01) << 5) +
                     ((GetCanInVal(28) & 0x01) << 4) + ((GetCanInVal(27) & 0x01) << 3) + ((GetCanInVal(26) & 0x01) << 2) +
                     ((GetCanInVal(25) & 0x01) << 1) + (GetCanInVal(24) & 0x01);
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