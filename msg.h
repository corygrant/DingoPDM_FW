#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"
#include "mailbox.h"
#include "dingopdm_config.h"    

CANTxFrame TxMsg0();
CANTxFrame TxMsg1();
CANTxFrame TxMsg2();
CANTxFrame TxMsg3();
CANTxFrame TxMsg4();
CANTxFrame TxMsg5();

[[maybe_unused]] static CANTxFrame (*TxMsgs[PDM_NUM_TX_MSGS])() = {
    TxMsg0,
    TxMsg1,
    TxMsg2,
    TxMsg3,
    TxMsg4,
    TxMsg5};

class InfoMsg
{
public:
    InfoMsg(MsgType type, MsgSrc src)
        : m_type(type), m_src(src){
          };

    /*
     * Send a message if the trigger is true and the message hasn't been sent yet
     */

    void Check(bool bTrigger, uint16_t nId, uint16_t nData0, uint16_t nData1, uint16_t nData2);

private:
    const MsgType m_type;
    const MsgSrc m_src;

    bool bSent;
    bool bLastTrig;
};