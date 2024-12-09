#pragma once

#include <cstdint>
#include "port.h"
#include "config.h"
#include "enums.h"

class Profet
{
public:
    Profet(int num, ProfetModel model, ioline_t in, ioline_t den, ioline_t dsel, AnalogChannel ain)
        : m_num(num), m_model(model), m_in(in), m_den(den), m_dsel(dsel), m_ain(ain)
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

    uint16_t nOutput;

    void Update(bool bOutEnabled);
    uint16_t GetCurrent() { return nCurrent; }
    ProfetState GetState() { return eState; }
    uint16_t GetOcCount() { return nOcCount; }

    void SetConfig(Config_Output *config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
    {
        pConfig = config;
        pInput = pVarMap[config->nInput];
    }

private:
    const uint16_t m_num;
    const ProfetModel m_model;
    const ioline_t m_in;
    const ioline_t m_den;
    const ioline_t m_dsel;
    const AnalogChannel m_ain;

    Config_Output *pConfig;

    uint16_t *pInput;

    ProfetState eState;
    ProfetState eReqState;
    ProfetState eLastState;

    uint16_t nCurrent; // Scaled current value (amps)
    uint16_t nIS;      // Raw analog current value
    float fKILIS;      // Current scaling factor

    bool bInRushActive;
    uint32_t nInRushOnTime;

    uint16_t nOcCount;       // Number of overcurrents
    uint32_t nOcTriggerTime; // Time of overcurrent
};