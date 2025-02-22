#pragma once

#include <cstdint>
#include "port.h"
#include "config.h"
#include "enums.h"

class Profet
{
public:
    Profet(int num, ProfetModel model, ioline_t in, ioline_t den, ioline_t dsel, AnalogChannel ain, PWMDriver *pwm, PwmChannel pwmCh)
        : m_num(num), m_model(model), m_in(in), m_den(den), m_dsel(dsel), m_ain(ain), m_pwmDriver(pwm), m_pwmChannel(pwmCh)
    {
        // Always on
        palSetLine(m_den);

        switch (model)
        {
        case ProfetModel::BTS7002_1EPP:
            fKILIS = BTS7002_1EPP_KILIS;
            break;
        case ProfetModel::BTS7008_2EPA_CH1:
        case ProfetModel::BTS7008_2EPA_CH2:
            fKILIS = BTS7008_2EPA_KILIS;
            break;
        case ProfetModel::BTS70012_1ESP:
            fKILIS = BTS70012_1ESP_KILIS;
            break;
        }
    }

    void SetConfig(Config_Output *config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

    void Update(bool bOutEnabled);
    uint16_t GetCurrent() { return nCurrent; }
    ProfetState GetState() { return eState; }
    uint16_t GetOcCount() { return nOcCount; }
    void SetDutyCycle(uint8_t nDC)
    {
        if (nDC > 100)
            nDC = 100;

        //PWM duty cycle is 0-10000
        // 100% = 10000
        // 50% = 5000
        // 0% = 0
        nDutyCycle = (nDC * 100);
    };
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);

    uint16_t nOutput;

private:
    const uint16_t m_num;
    const ProfetModel m_model;
    const ioline_t m_in;
    const ioline_t m_den;
    const ioline_t m_dsel;
    const AnalogChannel m_ain;
    PWMDriver *m_pwmDriver;
    const PwmChannel m_pwmChannel;

    Config_Output *pConfig;

    uint16_t *pInput;

    ProfetState eState;
    ProfetState eReqState;
    ProfetState eLastState;

    uint16_t nCurrent; // Scaled current value (amps)
    uint16_t nIS;      // Raw analog current value
    uint16_t nLastIS = 0; // Last analog current value
    float fKILIS;      // Current scaling factor

    bool bInRushActive;
    uint32_t nInRushOnTime;

    uint16_t nOcCount;       // Number of overcurrents
    uint32_t nOcTriggerTime; // Time of overcurrent

    uint16_t nDutyCycle = 0;
};