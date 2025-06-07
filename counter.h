#pragma once

#include <cstdint>
#include "config.h"
#include "dingopdm_config.h"

extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

class Counter
{
public:
    Counter() {

    };

    void SetConfig(Config_Counter* config)
    {
        pConfig = config;
        pIncInput = pVarMap[config->nIncInput];
        pDecInput = pVarMap[config->nDecInput];
        pResetInput = pVarMap[config->nResetInput];
    }

    void Update();
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_Counter *config);

    uint16_t nVal;

private:
    Config_Counter* pConfig;
    
    uint16_t *pIncInput;
    uint16_t *pDecInput;
    uint16_t *pResetInput;

    bool bLastInc;
    bool bLastDec;
    bool bLastReset;
};
