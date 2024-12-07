#include "can.h"
#include "hal.h"
#include "port.h"
#include "analog.h"
#include "input.h"
#include "dingopdm_config.h"
#include "mailbox.h"
#include "msg.h"

#include <iterator>


uint32_t nLastCanRxTime;

static THD_WORKING_AREA(waCanCyclicTxThread, 128);
void CanCyclicTxThread(void*)
{
    chRegSetThreadName("CAN Cyclic Tx");

    CANTxFrame msg;

    while(1)
    {
        for(uint8_t i=0; i<PDM_NUM_TX_MSGS; i++)
        {
            msg = TxMsgs[i]();
            msg.IDE = CAN_IDE_STD;
            msg.RTR = CAN_RTR_DATA;
            PostTxFrame(&msg);
        }
        
        chThdSleepMilliseconds(CAN_TX_CYCLIC_MSG_DELAY);
    }
}

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void*)
{
    chRegSetThreadName("CAN Tx");

    CANTxFrame msg;

    while(1)
    {
        //Send all messages in the TX queue
        msg_t res;
        do
        {
            res = FetchTxFrame(&msg);
            if (res == MSG_OK)
            {
                msg.IDE = CAN_IDE_STD;
                msg.RTR = CAN_RTR_DATA;
                bool txOk = canTryTransmitI(&CAND1, CAN_ANY_MAILBOX, &msg);
                if (txOk) //Returns true if mailbox full
                    PostTxFrame(&msg);

                chThdSleepMilliseconds(CAN_TX_MSG_SPLIT); 
            }
        } while (res == MSG_OK);
    }
}

static THD_WORKING_AREA(waCanRxThread, 128);
void CanRxThread(void*)
{
    CANRxFrame msg;

    chRegSetThreadName("CAN Rx");

    while (true)
    {
        msg_t res = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &msg, TIME_INFINITE);
        if (res != MSG_OK)
        {
            continue;
        }

        nLastCanRxTime = chVTGetSystemTimeX();

        res = PostRxFrame(&msg);
        //TODO:What to do if mailbox is full?
    }
}

void InitCan()
{
    //No rx filtering, need to evaluate all incoming messages
    canStart(&CAND1, &GetCanConfig());
    chThdCreateStatic(waCanCyclicTxThread, sizeof(waCanCyclicTxThread), NORMALPRIO + 1, CanCyclicTxThread, nullptr);
    chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO + 1, CanTxThread, nullptr);
    chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO + 1, CanRxThread, nullptr);
}

uint32_t GetLastCanRxTime()
{
    return nLastCanRxTime;
}