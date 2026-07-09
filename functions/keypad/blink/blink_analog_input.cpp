#include "blink_analog_input.h"

bool BlinkAnalogInput::Update(uint16_t nRawVal)
{
    // Scale raw value (0-500) to 0-5.0V range
    fVal = (static_cast<float>(nRawVal) / 100.0f);

    return true;
}