#include "counter.h"
#include "dbc.h"
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
                conf->stCounter[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 16, 1);
                conf->stCounter[nIndex].bWrapAround = Dbc::DecodeInt(rx->data8, 17, 1);
                conf->stCounter[nIndex].nIncInput = Dbc::DecodeInt(rx->data8, 24, 8);
                conf->stCounter[nIndex].nDecInput = Dbc::DecodeInt(rx->data8, 32, 8);
                conf->stCounter[nIndex].nResetInput = Dbc::DecodeInt(rx->data8, 40, 8);
                conf->stCounter[nIndex].nMinCount = Dbc::DecodeInt(rx->data8, 48, 4);
                conf->stCounter[nIndex].nMaxCount = Dbc::DecodeInt(rx->data8, 52, 4);
                conf->stCounter[nIndex].eIncEdge = static_cast<InputEdge>(Dbc::DecodeInt(rx->data8, 56, 2));
                conf->stCounter[nIndex].eDecEdge = static_cast<InputEdge>(Dbc::DecodeInt(rx->data8, 58, 2));
                conf->stCounter[nIndex].eResetEdge = static_cast<InputEdge>(Dbc::DecodeInt(rx->data8, 60, 2));
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++) tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::Counters) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].bEnabled, 16, 1);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].bWrapAround, 17, 1);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nIncInput, 24, 8);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nDecInput, 32, 8);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nResetInput, 40, 8);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nMinCount, 48, 4);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nMaxCount, 52, 4);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCounter[nIndex].eIncEdge), 56, 2);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCounter[nIndex].eDecEdge), 58, 2);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCounter[nIndex].eResetEdge), 60, 2);

            if(rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

void Counter::SetDefaultConfig(Config_Counter *config)
{
    config->bEnabled = false;
    config->nIncInput = 0;
    config->nDecInput = 0;
    config->nResetInput = 0;
    config->nMinCount = 0;
    config->nMaxCount = 4;
    config->eIncEdge = InputEdge::Rising;
    config->eDecEdge = InputEdge::Rising;
    config->eResetEdge = InputEdge::Rising;
    config->bWrapAround = false;
}