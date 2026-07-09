#pragma once

#include <cstdint>

class BlinkAnalogInput
{
public:
    BlinkAnalogInput() {};
    
    bool Update(uint16_t nRawVal);

    float fVal;

private:

};