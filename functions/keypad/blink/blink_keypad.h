#pragma once

#include "hal.h"

class Keypad;

bool CheckMsgBlinkMarine(Keypad* kp, CANRxFrame frame);
CANTxFrame GetTxMsgBlinkMarine(Keypad* kp, uint8_t nIndex);
CANTxFrame GetStartMsgBlinkMarine(Keypad* kp);
void SetModelBlinkMarine(Keypad* kp);
