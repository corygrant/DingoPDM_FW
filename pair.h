#pragma once

#include "hal.h"
#include "config.h"

MsgCmdResult Pair_ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx);