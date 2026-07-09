#pragma once

#include "port.h"
#include "enums.h"
#include "input.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_VirtualInput{
  bool bEnabled;
  bool bNot0;
  uint16_t nVar0;
  BoolOperator eCond0;
  bool bNot1;
  uint16_t nVar1;
  BoolOperator eCond1;
  bool bNot2;
  uint16_t nVar2;
  InputMode eMode;
};

class VirtualInput
{
public:
    VirtualInput()
    {
    };

    static const uint16_t nBaseIndex = 0x1400;

    void SetConfig(Config_VirtualInput *config)
    {
        pConfig = config;
        pVar0 = pVarMap[config->nVar0];
        pVar1 = pVarMap[config->nVar1];
        pVar2 = pVarMap[config->nVar2];
    }

    void Update();

    float fVal;

private:
    Config_VirtualInput *pConfig;

    Input input;

    float *pVar0;
    float *pVar1;
    float *pVar2;

    bool bResult0;
    bool bResult1;
    bool bResult2;
    bool bResultSec0;
    bool bResultSec1;
};