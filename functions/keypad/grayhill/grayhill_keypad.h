#pragma once

#include "hal.h"

class Keypad;

bool CheckMsgGrayhill(Keypad* kp, CANRxFrame frame);
CANTxFrame GetTxMsgGrayhill(Keypad* kp, uint8_t nIndex);
CANTxFrame GetStartMsgGrayhill(Keypad* kp);
void SetModelGrayhill(Keypad* kp);