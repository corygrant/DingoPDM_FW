#pragma once

#define LED_BLINK_SPLIT 200
#define LED_BLINK_PAUSE 1500

#include "port.h"

class Led
{
public:
    Led(LedType type)
    {
        switch(type)
        {
            case LedType::Status:
                m_line = LINE_LED_STATUS;
                break;
            case LedType::Error:
                m_line = LINE_LED_ERROR;
                break;
        }
    };

    void Solid(bool bState);
    void Code(uint32_t nTimeNow, uint8_t nCode);
    void Blink(uint32_t nTimeNow);

    private:
        ioline_t m_line;

        bool bState; //Current state of the LED
        uint32_t nUntil;
        uint8_t nBlinkCount;
        uint8_t nBlinkState; //0 = blinking code, 1 = pause between blinks
};