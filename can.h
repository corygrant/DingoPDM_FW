#pragma once

#include <cstdint>
#include "enums.h"
#include "config.h"

msg_t InitCan(CanBitrate bitrate);
void StopCan();
void ClearCanFilters();
void SetCanFilterId(uint8_t nFilterNum, uint32_t nId, bool bExtended);
uint32_t GetLastCanRxTime();
MsgCmdResult CanProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx);