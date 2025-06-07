#pragma once

#include "port.h"
#include "config.h"
#include "input.h"

extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

class VirtualInput
{
public:
    VirtualInput()
    {
    };

    void SetConfig(Config_VirtualInput *config)
    {
        pConfig = config;
        pVar0 = pVarMap[config->nVar0];
        pVar1 = pVarMap[config->nVar1];
        pVar2 = pVarMap[config->nVar2];
    }

    void Update();
    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_VirtualInput *config);

    uint16_t nVal;

private:
    Config_VirtualInput *pConfig;

    Input input;

    uint16_t *pVar0;
    uint16_t *pVar1;
    uint16_t *pVar2;

    bool bResult0;
    bool bResult1;
    bool bResult2;
    bool bResultSec0;
    bool bResultSec1;
};