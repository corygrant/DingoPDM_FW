#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"
#include "mailbox.h"
#include "device_config.h"    

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

[[maybe_unused]] static CANTxMsg (*TxMsgs[NUM_TX_MSGS])() = {
    TxMsg0,
    TxMsg1,
    TxMsg2,
    TxMsg3,
    TxMsg4,
    TxMsg5,
    TxMsg6,
    TxMsg7,
    TxMsg8};

