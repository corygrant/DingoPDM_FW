#pragma once

#include "port.h"
#include "hal.h"
#include "input.h"
#include "enums.h"

struct Config_CanInput{
  bool bEnabled;
  bool bTimeoutEnabled;
  uint16_t nTimeout; //ms
  uint8_t nIDE; //0=STD, 1=EXT
  uint32_t nID;
  uint8_t nStartBit;   
  uint8_t nBitLength;  
  float fFactor;       
  float fOffset;       
  ByteOrder eByteOrder;
  bool bSigned;        
  Operator eOperator;
  float fOperand;       
  InputMode eMode;
};

class CanInput
{
public:
    CanInput() {
    };

    static const uint16_t nBaseIndex = 0x1300;

    bool CheckMsg(CANRxFrame frame);

    void SetConfig(Config_CanInput* config) { pConfig = config; }
    void CheckTimeout();

    float fOutput;
    float fVal;

private:
    Config_CanInput* pConfig;

    Input input;

    uint32_t nLastRxTime;
};