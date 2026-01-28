#pragma once

#include <cstdint>
#include "config.h"

MsgCmd ConfigHandler(CANRxFrame *frame);

void ApplyAllConfig();
void ApplyConfig(MsgCmd eCmd);