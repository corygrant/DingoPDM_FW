#pragma once

#include <cstdint>
#include "enums.h"
#include "config.h"

msg_t InitCan(CanBitrate bitrate);
uint32_t GetLastCanRxTime();
MsgCmdResult CanProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx);