#pragma once

#include "hal.h"
#include "port.h"

#if HAS_USB
msg_t InitUsb();
bool GetUsbConnected();
#endif