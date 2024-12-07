#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"
#include "mailbox.h"
#include "dingopdm_config.h"    

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
        : m_type(type), m_src(src){
          };

    /*
     * Send a message if the trigger is true and the message hasn't been sent yet
     */

    void Check(bool bTrigger, uint16_t nId, uint8_t *nData0, uint8_t *nData1, uint8_t *nData2);

private:
    const MsgType m_type;
    const MsgSrc m_src;

    bool bSent;
    bool bLastTrig;
};