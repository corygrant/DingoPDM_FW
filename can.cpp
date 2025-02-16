#include "can.h"
#include "hal.h"
#include "port.h"
#include "dingopdm_config.h"
#include "mailbox.h"
#include "msg.h"

#include <iterator>

uint32_t nLastCanRxTime;

static THD_WORKING_AREA(waCanCyclicTxThread, 128);
void CanCyclicTxThread(void *)
{
    chRegSetThreadName("CAN Cyclic Tx");

    CANTxFrame msg;

    while (1)
    {
        for (uint8_t i = 0; i < PDM_NUM_TX_MSGS; i++)
        {
            msg = TxMsgs[i]();
            msg.IDE = CAN_IDE_STD;
            msg.RTR = CAN_RTR_DATA;
            PostTxFrame(&msg);
        }

        if (chThdShouldTerminateX())
            chThdExit(MSG_OK);

        chThdSleepMilliseconds(CAN_TX_CYCLIC_MSG_DELAY);
    }
}

static THD_WORKING_AREA(waCanTxThread, 256);
void CanTxThread(void *)
{
    chRegSetThreadName("CAN Tx");

    CANTxFrame msg;

    while (1)
    {
        // Send all messages in the TX queue
        msg_t res;
        do
        {
            res = FetchTxFrame(&msg);
            if (res == MSG_OK)
            {
                msg.IDE = CAN_IDE_STD;
                msg.RTR = CAN_RTR_DATA;
                bool txOk = canTryTransmitI(&CAND1, CAN_ANY_MAILBOX, &msg);
                if (txOk) // Returns true if mailbox full
                    PostTxFrame(&msg);

                chThdSleepMicroseconds(CAN_TX_MSG_SPLIT);
            }
        } while (res == MSG_OK);

        if (chThdShouldTerminateX())
            chThdExit(MSG_OK);

        chThdSleepMicroseconds(30);
    }
}

static THD_WORKING_AREA(waCanRxThread, 128);
void CanRxThread(void *)
{
    CANRxFrame msg;

    chRegSetThreadName("CAN Rx");

    while (true)
    {

        msg_t res = canReceiveTimeout(&CAND1, CAN_ANY_MAILBOX, &msg, TIME_IMMEDIATE);
        if (res == MSG_OK)
        {
            nLastCanRxTime = SYS_TIME;

            res = PostRxFrame(&msg);
            // TODO:What to do if mailbox is full?
        }

        if (chThdShouldTerminateX())
            chThdExit(MSG_OK);

        chThdSleepMicroseconds(30);
    }
}

static thread_t *canCyclicTxThreadRef;
static thread_t *canTxThreadRef;
static thread_t *canRxThreadRef;

void InitCan(CanBitrate bitrate)
{
    // No rx filtering, need to evaluate all incoming messages
    canStart(&CAND1, &GetCanConfig(bitrate));
    canCyclicTxThreadRef = chThdCreateStatic(waCanCyclicTxThread, sizeof(waCanCyclicTxThread), NORMALPRIO + 1, CanCyclicTxThread, nullptr);
    canTxThreadRef = chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO + 1, CanTxThread, nullptr);
    canRxThreadRef = chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO + 1, CanRxThread, nullptr);
}

uint32_t GetLastCanRxTime()
{
    return nLastCanRxTime;
}

MsgCmdResult CanProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 5 = Set CAN settings
    // DLC 1 = Get CAN settings

    if (rx->DLC == 5)
    {
        conf->stCanOutput.bEnabled = (rx->data8[1] & 0x02) >> 1;
        conf->stDevConfig.eCanSpeed = static_cast<CanBitrate>((rx->data8[1] & 0xF0) >> 4);
        conf->stCanOutput.nBaseId = (rx->data8[2] << 8) + rx->data8[3];
        conf->stCanOutput.nUpdateTime = rx->data8[4] * 10;
    }

    if ((rx->DLC == 5) ||
        (rx->DLC == 1))
    {
        tx->DLC = 5;
        tx->IDE = CAN_IDE_STD;

        tx->data8[0] = static_cast<uint8_t>(MsgCmd::Can) + 128;
        tx->data8[1] = ((static_cast<uint8_t>(conf->stDevConfig.eCanSpeed) & 0x0F) << 4) +
                      ((conf->stCanOutput.bEnabled & 0x01) << 1) + 1;
        tx->data8[2] = (conf->stCanOutput.nBaseId & 0xFF00) >> 8;
        tx->data8[3] = (conf->stCanOutput.nBaseId & 0x00FF);
        tx->data8[4] = (conf->stCanOutput.nUpdateTime) / 10;
        tx->data8[5] = 0;
        tx->data8[6] = 0;
        tx->data8[7] = 0;

        if (rx->DLC == 5)
            return MsgCmdResult::Write;
        else
            return MsgCmdResult::Request;
    }
    return MsgCmdResult::Invalid;
}