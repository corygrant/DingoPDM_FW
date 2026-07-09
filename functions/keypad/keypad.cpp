#include "keypad.h"
#include "blink_keypad.h"
#include "grayhill_keypad.h"

static THD_WORKING_AREA(waKeypadThread, 256);

static void KeypadThread(void *arg)
{
    Keypad *keypads = static_cast<Keypad *>(arg);

    chRegSetThreadName("Keypad");

    // Send start messages for all enabled keypads
    for (uint8_t k = 0; k < NUM_KEYPADS; k++)
    {
        if (!keypads[k].IsEnabled())
            continue;
        CANTxFrame msg = keypads[k].GetStartMsg();
        PostTxFrame(&msg);
        chThdSleepMilliseconds(KEYPAD_TX_MSG_SPLIT);
    }

    while (1)
    {
        for (uint8_t k = 0; k < NUM_KEYPADS; k++)
        {
            if (!keypads[k].IsEnabled())
                continue;

            if (!keypads[k].bStartMsgSent) {
                CANTxFrame startMsg = keypads[k].GetStartMsg();
                if (startMsg.SID != 0) 
                    PostTxFrame(&startMsg);   
                keypads[k].bStartMsgSent = true;
                chThdSleepMilliseconds(KEYPAD_TX_MSG_SPLIT);
            }

            for (uint8_t i = 0; i < keypads[k].GetNumTxMsgs(); i++)
            {
                CANTxFrame msg = keypads[k].GetTxMsg(i);
                if (msg.SID != 0)
                    PostTxFrame(&msg);
                chThdSleepMilliseconds(KEYPAD_TX_MSG_SPLIT);
            }
        }

        chThdSleepMilliseconds(KEYPAD_TX_MSG_DELAY);
    }
}

msg_t Keypad::InitThread(Keypad *keypads)
{
    chThdCreateStatic(waKeypadThread, sizeof(waKeypadThread), NORMALPRIO, KeypadThread, keypads);
    return MSG_OK;
}

void Keypad::SetConfig(Config_Keypad* config)
{
    pConfig = config;
    pDimmingInput = pVarMap[config->nDimmingVar];

    if (config->eModel <= KeypadModel::Blink15Key2Dial) {
        fnCheckMsg = CheckMsgBlinkMarine;
        fnGetTxMsg = GetTxMsgBlinkMarine;
        fnGetStartMsg = GetStartMsgBlinkMarine;
        fnSetModel = SetModelBlinkMarine;
        nNumTxMsgs = 4;
    } else {
        fnCheckMsg = CheckMsgGrayhill;
        fnGetTxMsg = GetTxMsgGrayhill;
        fnGetStartMsg = GetStartMsgGrayhill;
        fnSetModel = SetModelGrayhill;
        nNumTxMsgs = 2;
    }

    fnSetModel(this);
}

void Keypad::Update()
{
    CheckTimeout();

    for (uint8_t i = 0; i < nNumButtons; i++)
        button[i].UpdateLedState();
}

void Keypad::CheckTimeout()
{
    if (!pConfig->bEnabled)
        return;

    if (!pConfig->bTimeoutEnabled)
        return;

    if (SYS_TIME - nLastRxTime > pConfig->nTimeout)
    {
        // Timeout - reset values
        for (uint8_t i = 0; i < KEYPAD_MAX_BUTTONS; i++)
        {
            fButtonVal[i] = 0;
            fDialVal[i] = 0;
            fAnalogVal[i] = 0;
        }
    }
}

bool Keypad::CheckMsg(CANRxFrame frame)
{
    if (!pConfig->bEnabled || !fnCheckMsg)
        return false;
    return fnCheckMsg(this, frame);
}

CANTxFrame Keypad::GetTxMsg(uint8_t nIndex)
{
    if (!pConfig->bEnabled || !fnGetTxMsg)
        return {};

    if (pConfig->eModel != eLastModel) {
        fnSetModel(this);
        eLastModel = pConfig->eModel;
    }

    return fnGetTxMsg(this, nIndex);
}

CANTxFrame Keypad::GetStartMsg()
{
    if (!fnGetStartMsg)
        return {};
    return fnGetStartMsg(this);
}
