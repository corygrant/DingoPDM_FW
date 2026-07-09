#pragma once

#include "hal.h"
#include <cstdint>
#include "port.h"

#define PWM_UPDATE_TIME 2.0 //ms

#if NUM_OUTPUTS > 0

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_PwmOutput{
  bool bEnabled;
  bool bSoftStart;
  bool bVariableDutyCycle;
  uint16_t nDutyCycleInput;
  uint8_t nFixedDutyCycle;
  uint16_t nFreq;
  uint16_t nSoftStartRampTime; //ms
  uint16_t nDutyCycleInputDenom;
  uint16_t nMinDutyCycle;
};

class Pwm
{
public:
    Pwm(PWMDriver *pwm, const PWMConfig *pwmCfg, PwmChannel pwmCh)
        : m_pwm(pwm), m_pwmCfg(pwmCfg), m_pwmCh(pwmCh)
    {
    }

    static const uint16_t nBaseIndex = 0x1100;

    void SetConfig(Config_PwmOutput *config)
    {
        pConfig = config;
        pInput = pVarMap[config->nDutyCycleInput];

        if ((pConfig->bEnabled) && (m_pwm->state != PWM_READY))
            Init();
    }

    void Update();

    bool IsEnabled(){return pConfig->bEnabled;};

    uint8_t GetDutyCycle()
    {
        if (pConfig->bEnabled)
            return nDutyCycle;

        return 0;
    };

    void SetDutyCycle(uint16_t nDC)
    {
        if (nDC > 100)
            nDC = 100;

        if (nDC < pConfig->nMinDutyCycle)
            nDC = pConfig->nMinDutyCycle;

        nDutyCycle = nDC;
    };

    
    void On();
    void Off();
    void EnsureStarted();
    void OverrideFrequency(uint16_t nFreq);

private:
    PWMDriver *m_pwm;
    const PWMConfig *m_pwmCfg;
    PwmChannel m_pwmCh;

    float *pInput;

    Config_PwmOutput *pConfig;

    msg_t Init();
    uint8_t GetTargetDutyCycle();
    void InitSoftStart();
    void UpdateSoftStart();
    void UpdateFrequency();

    uint16_t nDutyCycle;
    uint16_t nLastFreq;

    bool bChannelEnabled;
    bool bLastChannelEnabled;
    bool bSoftStartComplete;
    float fSoftStartStep;
    
    uint32_t nSoftStartTime;
    uint32_t nSoftStartEndTime;
};
#endif