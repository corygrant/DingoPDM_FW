#include "counter.h"
#include "dbc.h"
#include "edge.h"

void Counter::Update()
{
    if (!pConfig->bEnabled)
    {
        fVal = 0;
        return;
    }

    if (Edge::Check(pConfig->eResetEdge, bLastReset, *pResetInput))
    {
        fVal = 0;
        return;
    }

    if (Edge::Check(pConfig->eIncEdge, bLastInc, *pIncInput))
    {
        fVal++;
        if (fVal > pConfig->nMaxCount)
        {
            fVal = pConfig->bWrapAround ? 0 : pConfig->nMaxCount;
        }
    }

    if (Edge::Check(pConfig->eDecEdge, bLastDec, *pDecInput))
    {
        if(fVal == 0)
        {
            fVal = pConfig->bWrapAround ? pConfig->nMaxCount : pConfig->nMinCount;
        }
        else
        {
            fVal--;
        }
    }

    bLastInc = *pIncInput;
    bLastDec = *pDecInput;
    bLastReset = *pResetInput;
}

MsgCmdResult CounterMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 4 = Set counter settings (without inputs)
    // DLC 2 = Get counter settings

    if ((rx->DLC == 4) || (rx->DLC == 2))
    {
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_COUNTERS)
        {
            if (rx->DLC == 4)
            {
                conf->stCounter[nIndex].bEnabled = Dbc::DecodeInt(rx->data8, 16, 1);
                conf->stCounter[nIndex].bWrapAround = Dbc::DecodeInt(rx->data8, 17, 1);
                conf->stCounter[nIndex].nMinCount = Dbc::DecodeInt(rx->data8, 18, 4);
                conf->stCounter[nIndex].nMaxCount = Dbc::DecodeInt(rx->data8, 22, 4);
                conf->stCounter[nIndex].eIncEdge = static_cast<InputEdge>(Dbc::DecodeInt(rx->data8, 26, 2));
                conf->stCounter[nIndex].eDecEdge = static_cast<InputEdge>(Dbc::DecodeInt(rx->data8, 28, 2));
                conf->stCounter[nIndex].eResetEdge = static_cast<InputEdge>(Dbc::DecodeInt(rx->data8, 30, 2));
            }

            tx->DLC = 4;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++) tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::Counters) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].bEnabled, 16, 1);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].bWrapAround, 17, 1);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nMinCount, 18, 4);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nMaxCount, 22, 4);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCounter[nIndex].eIncEdge), 26, 2);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCounter[nIndex].eDecEdge), 28, 2);
            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(conf->stCounter[nIndex].eResetEdge), 30, 2);

            if (rx->DLC == 4)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

MsgCmdResult CounterInputsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set counter inputs
    // DLC 2 = Get counter inputs

    if ((rx->DLC == 8) || (rx->DLC == 2))
    {
        uint8_t nIndex = rx->data8[1];

        if (nIndex < PDM_NUM_COUNTERS)
        {
            if (rx->DLC == 8)
            {
                conf->stCounter[nIndex].nIncInput = Dbc::DecodeInt(rx->data8, 16, 16);
                conf->stCounter[nIndex].nDecInput = Dbc::DecodeInt(rx->data8, 32, 16);
                conf->stCounter[nIndex].nResetInput = Dbc::DecodeInt(rx->data8, 48, 16);
            }

            tx->DLC = 8;
            tx->IDE = CAN_IDE_STD;

            for (int i = 0; i < 8; i++) tx->data8[i] = 0;

            Dbc::EncodeInt(tx->data8, static_cast<uint8_t>(MsgCmd::CountersInputs) + 128, 0, 8);
            Dbc::EncodeInt(tx->data8, nIndex, 8, 8);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nIncInput, 16, 16);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nDecInput, 32, 16);
            Dbc::EncodeInt(tx->data8, conf->stCounter[nIndex].nResetInput, 48, 16);

            if (rx->DLC == 8)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }

        return MsgCmdResult::Invalid;
    }

    return MsgCmdResult::Invalid;
}

MsgCmdResult Counter::ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx)
{
    MsgCmd cmd = static_cast<MsgCmd>(rx->data8[0]);

    if (cmd == MsgCmd::Counters)
        return CounterMsg(conf, rx, tx);
    else if (cmd == MsgCmd::CountersInputs)
        return CounterInputsMsg(conf, rx, tx);

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