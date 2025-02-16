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
CANTxFrame TxMsg6();

[[maybe_unused]] static CANTxFrame (*TxMsgs[PDM_NUM_TX_MSGS])() = {
    TxMsg0,
    TxMsg1,
    TxMsg2,
    TxMsg3,
    TxMsg4,
    TxMsg5,
    TxMsg6};

class InfoMsg
{
public:
    InfoMsg(MsgType type = MsgType::Info, MsgSrc src = MsgSrc::Init)
        : m_type(type), m_src(src){
          };

    InfoMsg& operator=(const InfoMsg& other){
        if (this != &other)
        {
            m_type = other.m_type;
            m_src = other.m_src;
        }
        return *this;
    }

    /*
     * Send a message if the trigger is true and the message hasn't been sent yet
     */

    void Check(bool bTrigger, uint16_t nId, uint16_t nData0, uint16_t nData1, uint16_t nData2);

private:
    MsgType m_type;
    MsgSrc m_src;

    bool bSent;
    bool bLastTrig;
};