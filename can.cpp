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
                bool txOk = canTryTransmitI(&CAND1, CAN_ANY_MAILBOX, &msg);
                if (txOk) //Returns true if mailbox full
                    PostTxFrame(&msg);

                chThdSleepMilliseconds(CAN_TX_MSG_SPLIT); 
            }
        } while (res == MSG_OK);
        
        //Send cyclic messages
        for(uint8_t i=0; i<PDM_NUM_TX_MSGS; i++)
        {
            msg = GetTxMsgs[i]();
            canTryTransmitI(&CAND1, CAN_ANY_MAILBOX, &msg); //Don't care if msg isn't sent
            chThdSleepMilliseconds(CAN_TX_MSG_SPLIT);    
        }
        
        chThdSleepMilliseconds(CAN_TX_MSG_DELAY);
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
    chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO + 1, CanTxThread, nullptr);
    chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO + 1, CanRxThread, nullptr);
}

uint32_t GetLastCanRxTime()
{
    return nLastCanRxTime;
}