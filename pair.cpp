#include "pair.h"

MsgCmdResult Pair_ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx)
{
    // DLC 8 = Set output pair settings
    // DLC 2 = Get output pair settings

    if ((rx->DLC == 5) ||
        (rx->DLC == 2))
    {
        uint8_t nIndex = (rx->data8[1] & 0xF0) >> 4;
        if (nIndex < PDM_NUM_OUTPUTS)
        {
            if (rx->DLC == 5)
            {
                conf->stOutput[nIndex].stPair.eMode = static_cast<PairOutputMode>(rx->data8[2]);
                conf->stOutput[nIndex].stPair.nPairOutNum = rx->data8[3];
                conf->stOutput[nIndex].stPair.bUsePwm = (rx->data8[4] & 0x01);
            }

            tx->DLC = 5;
            tx->IDE = CAN_IDE_STD;

            tx->data8[0] = static_cast<uint8_t>(MsgCmd::OutputsPair) + 128;
            tx->data8[1] = ((nIndex & 0x0F) << 4);
            tx->data8[2] = static_cast<uint8_t>(conf->stOutput[nIndex].stPair.eMode);
            tx->data8[3] = conf->stOutput[nIndex].stPair.nPairOutNum;
            tx->data8[4] = conf->stOutput[nIndex].stPair.bUsePwm;

            if (rx->DLC == 5)
                return MsgCmdResult::Write;
            else
                return MsgCmdResult::Request;
        }
        return MsgCmdResult::Invalid;
    }
    return MsgCmdResult::Invalid;
}