#include "msg.h"
#include "config.h"
#include "profet.h"
#include "digital.h"
#include "pdm.h"
#include "analog.h"

CANTxFrame GetMsg0()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 0 (Digital inputs 1-2) and Device Status
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 0;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = (in[1].nVal << 1) + in[0].nVal;
    stMsg.data8[1] = static_cast<uint8_t>(GetPdmState()) + (PDM_TYPE << 4);
    stMsg.data16[1] = (uint16_t)GetTotalCurrent();
    stMsg.data16[2] = (uint16_t)(GetBattVolt() * 10);
    stMsg.data16[3] = (uint16_t)GetBoardTemp();

    return stMsg;
}

CANTxFrame GetMsg1()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 1 (Out 1-4 Current)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 1;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data16[0] = pf[0].GetCurrent();
    stMsg.data16[1] = pf[1].GetCurrent();
    stMsg.data16[2] = pf[2].GetCurrent();
    stMsg.data16[3] = pf[3].GetCurrent();

    return stMsg;
}

CANTxFrame GetMsg2()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 2 (Out 5-8 Current)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 2;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data16[0] = pf[4].GetCurrent();
    stMsg.data16[1] = pf[5].GetCurrent();
    stMsg.data16[2] = pf[6].GetCurrent();
    stMsg.data16[3] = pf[7].GetCurrent();

    return stMsg;
}

CANTxFrame GetMsg3()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 3 (Out 1-8 Status)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 3;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = (static_cast<uint8_t>(pf[1].GetState()) << 4) + static_cast<uint8_t>(pf[0].GetState());
    stMsg.data8[1] = (static_cast<uint8_t>(pf[3].GetState()) << 4) + static_cast<uint8_t>(pf[2].GetState());
    stMsg.data8[2] = (static_cast<uint8_t>(pf[5].GetState()) << 4) + static_cast<uint8_t>(pf[4].GetState());
    stMsg.data8[3] = (static_cast<uint8_t>(pf[7].GetState()) << 4) + static_cast<uint8_t>(pf[6].GetState());
    stMsg.data32[1] = 3;
    //stMsg.data8[4] = (*pVariableMap[60] << 1) + *pVariableMap[59];
    //stMsg.data8[5] = (stWiper.eState << 4) + stWiper.eSelectedSpeed;
    //stMsg.data8[6] = ((nFlashers[3] & 0x01) << 3) + ((nFlashers[2] & 0x01) << 2) +
    //                 ((nFlashers[1] & 0x01) << 1) + (nFlashers[0] & 0x01) +
    //                 ((*stPdmConfig.stFlasher[3].pInput & 0x01) << 7) + ((*stPdmConfig.stFlasher[2].pInput & 0x01) << 6) +
    //                 ((*stPdmConfig.stFlasher[1].pInput & 0x01) << 5) + ((*stPdmConfig.stFlasher[0].pInput & 0x01) << 4);
    //stMsg.data8[7] = 0;

    return stMsg;
}

CANTxFrame GetMsg4()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 4 (Out 1-8 Reset Count)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 4;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data8[0] = pf[0].GetOcCount();
    stMsg.data8[1] = pf[1].GetOcCount();
    stMsg.data8[2] = pf[2].GetOcCount();
    stMsg.data8[3] = pf[3].GetOcCount();
    stMsg.data8[4] = pf[4].GetOcCount();
    stMsg.data8[5] = pf[5].GetOcCount();
    stMsg.data8[6] = pf[6].GetOcCount();
    stMsg.data8[7] = pf[7].GetOcCount();

    return stMsg;
}

CANTxFrame GetMsg5()
{
    CANTxFrame stMsg;
    //=======================================================
    // Build Msg 5 (CAN Inputs and Virtual Inputs)
    //=======================================================
    stMsg.SID = stConfig.stCanOutput.nBaseId + 5;
    stMsg.DLC = 8; // Bytes to send
    stMsg.data64[0] = 5;
    /*
    stMsg.data8[0] = ((nCanInputs[7] & 0x01) << 7) + ((nCanInputs[6] & 0x01) << 6) + ((nCanInputs[5] & 0x01) << 5) +
                     ((nCanInputs[4] & 0x01) << 4) + ((nCanInputs[3] & 0x01) << 3) + ((nCanInputs[2] & 0x01) << 2) +
                     ((nCanInputs[1] & 0x01) << 1) + (nCanInputs[0] & 0x01);
    stMsg.data8[1] = ((nCanInputs[15] & 0x01) << 7) + ((nCanInputs[14] & 0x01) << 6) + ((nCanInputs[13] & 0x01) << 5) +
                     ((nCanInputs[12] & 0x01) << 4) + ((nCanInputs[11] & 0x01) << 3) + ((nCanInputs[10] & 0x01) << 2) +
                     ((nCanInputs[9] & 0x01) << 1) + (nCanInputs[8] & 0x01);
    stMsg.data8[2] = ((nCanInputs[23] & 0x01) << 7) + ((nCanInputs[22] & 0x01) << 6) + ((nCanInputs[21] & 0x01) << 5) +
                     ((nCanInputs[20] & 0x01) << 4) + ((nCanInputs[19] & 0x01) << 3) + ((nCanInputs[18] & 0x01) << 2) +
                     ((nCanInputs[17] & 0x01) << 1) + (nCanInputs[16] & 0x01);
    stMsg.data8[3] = ((nCanInputs[31] & 0x01) << 7) + ((nCanInputs[30] & 0x01) << 6) + ((nCanInputs[29] & 0x01) << 5) +
                     ((nCanInputs[28] & 0x01) << 4) + ((nCanInputs[27] & 0x01) << 3) + ((nCanInputs[26] & 0x01) << 2) +
                     ((nCanInputs[25] & 0x01) << 1) + (nCanInputs[24] & 0x01);
    stMsg.data8[4] = ((nVirtInputs[7] & 0x01) << 7) + ((nVirtInputs[6] & 0x01) << 6) + ((nVirtInputs[5] & 0x01) << 5) +
                     ((nVirtInputs[4] & 0x01) << 4) + ((nVirtInputs[3] & 0x01) << 3) + ((nVirtInputs[2] & 0x01) << 2) +
                     ((nVirtInputs[1] & 0x01) << 1) + (nVirtInputs[0] & 0x01);
    stMsg.data8[5] = ((nVirtInputs[15] & 0x01) << 7) + ((nVirtInputs[14] & 0x01) << 6) + ((nVirtInputs[13] & 0x01) << 5) +
                     ((nVirtInputs[12] & 0x01) << 4) + ((nVirtInputs[11] & 0x01) << 3) + ((nVirtInputs[10] & 0x01) << 2) +
                     ((nVirtInputs[9] & 0x01) << 1) + (nVirtInputs[8] & 0x01);
                     */
    //stMsg.data8[6] = 0;
    //stMsg.data8[7] = 0;

    return stMsg;
}