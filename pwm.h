#pragma once

#include "hal.h"
#include <cstdint>
#include "config.h"

class Pwm
{
public:
    Pwm(PWMDriver *pwm, const PWMConfig *pwmCfg, PwmChannel pwmCh)
        : m_pwm(pwm), m_pwmCfg(pwmCfg), m_pwmCh(pwmCh)
    {
    }

    void SetConfig(Config_PwmOutput *config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pInput = pVarMap[config->nDutyCycleInput];

        if (pConfig->bEnabled)
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

        nDutyCycle = nDC;
    };

    
    void On();
    void Off();

    static MsgCmdResult ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx);

private:
    PWMDriver *m_pwm;
    const PWMConfig *m_pwmCfg;
    PwmChannel m_pwmCh;

    uint16_t *pInput;

    Config_PwmOutput *pConfig;

    msg_t Init();

    uint16_t nDutyCycle;
    uint16_t nLastFreq;
};