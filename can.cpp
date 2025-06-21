#include "can.h"
#include "hal.h"
#include "port.h"
#include "dingopdm_config.h"
#include "mailbox.h"
#include "msg.h"

#include <iterator>

static CANFilter canfilters[STM32_CAN_MAX_FILTERS];
static uint32_t nFilterIds[STM32_CAN_MAX_FILTERS * 2];
static bool bFilterExtended[STM32_CAN_MAX_FILTERS * 2];

static uint32_t nLastCanRxTime;
static bool bCanFilterEnabled = true;

void ConfigureCanFilters();

static THD_WORKING_AREA(waCanCyclicTxThread, 128);
void CanCyclicTxThread(void *)
{
    chRegSetThreadName("CAN Cyclic Tx");

    CANTxMsg msg;

    while (1)
    {

        for (uint8_t i = 0; i < PDM_NUM_TX_MSGS; i++)
        {
            msg = TxMsgs[i]();
            if (!msg.bSend)
                continue;
            msg.frame.IDE = CAN_IDE_STD;
            msg.frame.RTR = CAN_RTR_DATA;
            PostTxFrame(&msg.frame);
        }

        //Post keypad messages

        //Post CANboard messages

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
                canTryTransmitI(&CAND1, CAN_ANY_MAILBOX, &msg);
                // Returns true if mailbox full or nothing connected
                // TODO: What to do if no tx?

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
            //palToggleLine(LINE_E2);
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

msg_t InitCan(Config_DeviceConfig *conf)
{
    if (canCyclicTxThreadRef || canTxThreadRef || canRxThreadRef)
    {
        StopCan();
    }

    SetCanFilterEnabled(conf->bCanFilterEnabled);

    ConfigureCanFilters();

    msg_t ret = canStart(&CAND1, &GetCanConfig(conf->eCanSpeed));
    if (ret != HAL_RET_SUCCESS)
        return ret;
    canCyclicTxThreadRef = chThdCreateStatic(waCanCyclicTxThread, sizeof(waCanCyclicTxThread), NORMALPRIO + 1, CanCyclicTxThread, nullptr);
    canTxThreadRef = chThdCreateStatic(waCanTxThread, sizeof(waCanTxThread), NORMALPRIO + 1, CanTxThread, nullptr);
    canRxThreadRef = chThdCreateStatic(waCanRxThread, sizeof(waCanRxThread), NORMALPRIO + 1, CanRxThread, nullptr);

    return HAL_RET_SUCCESS;
}

void StopCan()
{
    // Signal threads to terminate
    chThdTerminate(canCyclicTxThreadRef);
    chThdTerminate(canTxThreadRef);
    chThdTerminate(canRxThreadRef);

    // Wait for threads to exit
    chThdWait(canCyclicTxThreadRef);
    chThdWait(canTxThreadRef);
    chThdWait(canRxThreadRef);

    // Stop CAN driver
    canStop(&CAND1);

    // Reset thread references
    canCyclicTxThreadRef = NULL;
    canTxThreadRef = NULL;
    canRxThreadRef = NULL;
}

void ClearCanFilters()
{
    // Clear all filters
    for (uint8_t i = 0; i < STM32_CAN_MAX_FILTERS; i++)
    {
        nFilterIds[i] = 0;
        bFilterExtended[i] = false;

        canfilters[i].register1 = 0;
        canfilters[i].register2 = 0;
        canfilters[i].filter = 0;
        canfilters[i].assignment = 0;
        canfilters[i].mode = 0;
        canfilters[i].scale = 0;
    }
}

void SetCanFilterId(uint8_t nFilterNum, uint32_t nId, bool bExtended)
{
    if (nFilterNum >= (STM32_CAN_MAX_FILTERS * 2))
        return;

    bFilterExtended[nFilterNum] = bExtended;

    if (bExtended)
    {
        nFilterIds[nFilterNum] = (nId << 3) | 0x04; // Set IDE bit for extended ID
    }
    else
    {
        nFilterIds[nFilterNum] = nId << 21;
    }
}

void ConfigureCanFilters()
{

    if(!bCanFilterEnabled)
    {
        // Default HAL config = filter 0 enabled to allow all messages
        return;
    }

    uint8_t nCurrentFilter = 0;

    // Go through nFilterIds and set filter register1 and register2 for each filter if ID is set
    // CANNOT SET ALL FILTERS, MUST USE ONLY THE NUMBER OF REQUIRED FILTERS
    for (uint8_t i = 0; i < (STM32_CAN_MAX_FILTERS * 2); i += 2)
    {
        if (nFilterIds[i] != 0 || nFilterIds[i + 1] != 0)
        {
            canfilters[nCurrentFilter].filter = nCurrentFilter; // Filter bank number
            canfilters[nCurrentFilter].assignment = 0;          // Assign to FIFO 0
            canfilters[nCurrentFilter].mode = 1;                // List mode
            canfilters[nCurrentFilter].scale = 1;               // 32-bit scale

            // First ID (register1)
            if (nFilterIds[i] != 0)
            {
                canfilters[nCurrentFilter].register1 = nFilterIds[i];
            }

            // Second ID (register2)
            if (nFilterIds[i + 1] != 0)
            {
                canfilters[nCurrentFilter].register2 = nFilterIds[i + 1];
            }

            nCurrentFilter++;
        }
    }

    // Apply all filter configurations
    // If no CAN inputs are enabled, filter[0].register1 is still set to settings request message ID (BaseId-1)
    canSTM32SetFilters(&CAND1, STM32_CAN_MAX_FILTERS, nCurrentFilter, canfilters);
}

uint32_t GetLastCanRxTime()
{
    return nLastCanRxTime;
}

void SetCanFilterEnabled(bool bEnabled)
{
    bCanFilterEnabled = bEnabled;

    // TODO: Reconfigure filters if enabled/disabled
}

MsgCmdResult CanProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 4 = Set CAN settings
    // DLC 1 = Get CAN settings

    if (rx->DLC == 4)
    {
        conf->stDevConfig.bSleepEnabled = rx->data8[1] & 0x01;
        conf->stDevConfig.bCanFilterEnabled = (rx->data8[1] & 0x02) >> 1;
        conf->stDevConfig.eCanSpeed = static_cast<CanBitrate>((rx->data8[1] & 0xF0) >> 4);
        conf->stCanOutput.nBaseId = (rx->data8[2] << 8) + rx->data8[3];
    }

    if ((rx->DLC == 4) ||
        (rx->DLC == 1))
    {
        tx->DLC = 4;
        tx->IDE = CAN_IDE_STD;

        tx->data8[0] = static_cast<uint8_t>(MsgCmd::Can) + 128;
        tx->data8[1] = ((static_cast<uint8_t>(conf->stDevConfig.eCanSpeed) & 0x0F) << 4) +
                       ((conf->stDevConfig.bCanFilterEnabled & 0x01) << 1) +
                       (conf->stDevConfig.bSleepEnabled & 0x01);
        tx->data8[2] = (conf->stCanOutput.nBaseId & 0xFF00) >> 8;
        tx->data8[3] = (conf->stCanOutput.nBaseId & 0x00FF);
        tx->data8[4] = 0;
        tx->data8[5] = 0;
        tx->data8[6] = 0;
        tx->data8[7] = 0;

        SetCanFilterEnabled(conf->stDevConfig.bCanFilterEnabled);

        if (rx->DLC == 4)
            return MsgCmdResult::Write;
        else
            return MsgCmdResult::Request;
    }
    return MsgCmdResult::Invalid;
}