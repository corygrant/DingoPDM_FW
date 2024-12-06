#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"
#include "mailbox.h"

CANTxFrame GetMsg0();
CANTxFrame GetMsg1();
CANTxFrame GetMsg2();
CANTxFrame GetMsg3();
CANTxFrame GetMsg4();
CANTxFrame GetMsg5();

[[maybe_unused]] static CANTxFrame (*GetTxMsgs[PDM_NUM_TX_MSGS])() = {
    GetMsg0,
    GetMsg1,
    GetMsg2,
    GetMsg3,
    GetMsg4,
    GetMsg5};

class InfoMsg
{
public:
    InfoMsg(MsgType type, MsgSrc src)
        : m_type(type), m_src(src) {
          };

    /*
     * Send a message if the trigger is true and the message hasn't been sent yet
     */

    void Send(bool bTrigger, uint32_t nId, uint8_t *nData0, uint8_t *nData1, uint8_t *nData2)
    {
        if (!bTrigger)
        {
            bSent = false;
            return;
        }

        if (bSent)
            return;

        CANTxFrame tx;
        tx.DLC = 5;

        tx.data8[0] = static_cast<uint8_t>(m_type);
        tx.data8[1] = static_cast<uint8_t>(m_src);
        tx.data8[2] = *nData0;
        tx.data8[3] = *nData1;
        tx.data8[4] = *nData2;

        tx.SID = nId;
        PostTxFrame(&tx);

        bSent = true;
    }

private:
    const MsgType m_type;
    const MsgSrc m_src;

    bool bSent;
    bool bLastTrig;
};