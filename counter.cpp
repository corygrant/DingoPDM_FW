#include "counter.h"
#include "edge.h"

void Counter::Update()
{
    if (!pConfig->bEnabled)
    {
        nVal = 0;
        return;
    }

    if (Edge::Check(pConfig->eResetEdge, bLastReset, *pResetInput))
    {
        nVal = 0;
        return;
    }

    if (Edge::Check(pConfig->eIncEdge, bLastInc, *pIncInput))
    {
        nVal++;
        if (nVal > pConfig->nMaxCount)
        {
            nVal = pConfig->bWrapAround ? 0 : pConfig->nMaxCount;
        }
    }

    if (Edge::Check(pConfig->eDecEdge, bLastDec, *pDecInput))
    {
        if(nVal == 0)
        {
            nVal = pConfig->bWrapAround ? pConfig->nMaxCount : pConfig->nMinCount;
        }
        else
        {
            nVal--;
        }
    }

    bLastInc = *pIncInput;
    bLastDec = *pDecInput;
    bLastReset = *pResetInput;
}

MsgCmdResult Counter::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set counter settings
    // DLC 2 = Get counter settings

    if ((rx->DLC == 8) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_COUNTERS)
        {
            if (rx->DLC == 8)
            {
                conf->stCounter[nIndex].bEnabled = (rx->data8[2] & 0x01);
                conf->stCounter[nIndex].bWrapAround = ((rx->data8[2] & 0x02) >> 1);
                conf->stCounter[nIndex].nIncInput = rx->data8[3];
                conf->stCounter[nIndex].nDecInput = rx->data8[4];
                conf->stCounter[nIndex].nResetInput = rx->data8[5];
                conf->stCounter[nIndex].nMinCount = (rx->data8[6] & 0x0F);
                conf->stCounter[nIndex].nMaxCount = ((rx->data8[6] & 0xF0) >> 4);
                conf->stCounter[nIndex].eIncEdge = static_cast<InputEdge>(rx->data8[7] & 0x03);
                conf->stCounter[nIndex].eDecEdge = static_cast<InputEdge>((rx->data8[7] & 0x0C) >> 2);
                conf->stCounter[nIndex].eResetEdge = static_cast<InputEdge>((rx->data8[7] & 0x30) >> 4);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::Counters) + 128;
            tx->data8[1] = nIndex;
            tx->data8[2] = (conf->stCounter[nIndex].bEnabled & 0x01) +
                           ((conf->stCounter[nIndex].bWrapAround & 0x01) << 1);
            tx->data8[3] = conf->stCounter[nIndex].nIncInput;
            tx->data8[4] = conf->stCounter[nIndex].nDecInput;
            tx->data8[5] = conf->stCounter[nIndex].nResetInput;
            tx->data8[6] = ((conf->stCounter[nIndex].nMaxCount & 0x0F) << 4) +
                           (conf->stCounter[nIndex].nMinCount & 0x0F);
            tx->data8[7] = (static_cast<uint8_t>(conf->stCounter[nIndex].eIncEdge) & 0x03) +
                           ((static_cast<uint8_t>(conf->stCounter[nIndex].eDecEdge) & 0x03) << 2) +
                           ((static_cast<uint8_t>(conf->stCounter[nIndex].eResetEdge) & 0x03) << 4);

            if(rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}