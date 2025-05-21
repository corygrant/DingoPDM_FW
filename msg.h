#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"
#include "mailbox.h"
#include "dingopdm_config.h"    

struct CANTxMsg
{
    CANTxFrame frame;
    bool bSend;
};

CANTxMsg TxMsg0();
CANTxMsg TxMsg1();
CANTxMsg TxMsg2();
CANTxMsg TxMsg3();
CANTxMsg TxMsg4();
CANTxMsg TxMsg5();
CANTxMsg TxMsg6();
CANTxMsg TxMsg7();
CANTxMsg TxMsg8();
CANTxMsg TxMsg9();
CANTxMsg TxMsg10();
CANTxMsg TxMsg11();
CANTxMsg TxMsg12();
CANTxMsg TxMsg13();
CANTxMsg TxMsg14();
CANTxMsg TxMsg15();
CANTxMsg TxMsg16();
CANTxMsg TxMsg17();
CANTxMsg TxMsg18();

[[maybe_unused]] static CANTxMsg (*TxMsgs[PDM_NUM_TX_MSGS])() = {
    TxMsg0,
    TxMsg1,
    TxMsg2,
    TxMsg3,
    TxMsg4,
    TxMsg5,
    TxMsg6,
    TxMsg7,
    TxMsg8,
    TxMsg9,
    TxMsg10,
    TxMsg11,
    TxMsg12,
    TxMsg13,
    TxMsg14,
    TxMsg15,
    TxMsg16,
    TxMsg17,
    TxMsg18};

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