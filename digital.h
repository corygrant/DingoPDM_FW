#pragma once

#include "port.h"
#include "enums.h"
#include "config.h"
#include "input.h"

class Digital
{
public:
    Digital(DigitalChannel channel)
        : m_channel(channel)
    {
        switch (channel)
        {
        case DigitalChannel::In1:
            m_line = LINE_DI1;
            break;
        case DigitalChannel::In2:
            m_line = LINE_DI2;
            break;
        }
    }

    uint16_t nVal;

    void Update();

    void SetConfig(Config_Input *config)
    {
        pConfig = config;

        SetPull(config->ePull);
    }

private:
    const DigitalChannel m_channel;

    ioline_t m_line;

    void SetPull(InputPull pull);

    Config_Input *pConfig;

    Input input;

    bool bInit;
    bool bLast;
    bool bCheck;
    uint32_t nLastTrigTime;
};

extern Digital in[PDM_NUM_INPUTS];