#pragma once

#include <cstdint>
#include "port.h"
#include "enums.h"
#include "config.h"

class KeypadDial
{
public:
KeypadDial() {};

    void SetConfig(Config_KeypadDial *config)
    {
        pConfig = config;
    }

    static MsgCmdResult ProcessSettingsMsg(PdmConfig* conf, CANRxFrame *rx, CANTxFrame *tx);
    static void SetDefaultConfig(Config_KeypadDial *config);
    
    void Update(uint64_t data);

    void UpdateLeds();

    bool GetCW(){ return bClockwise;}
    bool GetCCW(){ return bCounterClockwise;}
    uint8_t GetTicks(){ return nTicks;}
    uint16_t GetEncoderTicks(){ return nEncoderTicks;}

    uint8_t GetMaxEncoderTicks(){ return nMaxEncoderTicks;}

private:
    Config_KeypadDial *pConfig;

    bool bClockwise;
    bool bCounterClockwise;
    uint8_t nTicks;

    uint16_t nEncoderTicks;

    uint8_t nMaxEncoderTicks;
};