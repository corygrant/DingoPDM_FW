#pragma once

#include "port.h"

#if HAS_NEOPIXELS

#include "neopixel.h"
#include "hal.h"

#define NEO_T0H  29   // 0.4µs @ 72MHz TIM1 clock
#define NEO_T1H  58   // 0.8µs @ 72MHz TIM1 clock
#define NEO_RST  50   // ≥50 periods of CCR=0 for reset pulse

class NeoPixels {
public:
    NeoPixels(uint8_t numPixels, PWMDriver *pwmDriver, const PWMConfig *pwmCfg, PwmChannel pwmCh);

    void Update();
    bool IsBusy() const { return m_busy; }

    NeoPixel pixels[MAX_NEOPIXELS] = {};

private:
    const uint8_t    m_numPixels;
    PWMDriver       *m_pwmDriver;
    const PWMConfig *m_pwmCfg;
    PwmChannel       m_pwmCh;

    // GRB bit stream + reset padding; entries are CCR values (T0H or T1H or 0)
    uint16_t m_dmaBuffer[MAX_NEOPIXELS * 24 + NEO_RST];

    const stm32_dma_stream_t *m_dmaStream;
    volatile bool m_busy;
    bool          m_initialized;

    void init();
    void buildBuffer();
    static void dmaCallback(void *p, uint32_t flags);
};

#endif
