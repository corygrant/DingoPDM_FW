#pragma once

#include <cstdint>
#include "enums.h"
#include "port.h"
#include "wiper_mode.h"
#include "wiper_digin.h"
#include "wiper_intin.h"
#include "wiper_mixin.h"

extern float *pVarMap[VAR_MAP_SIZE];

struct Config_Wiper{
  bool bEnabled;
  WiperMode eMode;
  uint16_t nSlowInput;   //WiperMode_DigIn
  uint16_t nFastInput;   //WiperMode_DigIn
  uint16_t nInterInput;  //WiperMode_DigIn
  uint16_t nOnInput;     //WiperMode_MixIn
  uint16_t nSpeedInput;  //WiperMode_IntIn and WiperMode_MixIn
  uint16_t nParkInput;
  bool bParkStopLevel;
  uint16_t nSwipeInput;
  uint16_t nWashInput;
  uint8_t nWashWipeCycles;
  WiperSpeed eSpeedMap[NUM_WIPER_SPEED_MAP];
  uint16_t nIntermitTime[NUM_WIPER_INTER_DELAYS]; //ms
};

class Wiper
{
    friend class Wiper_DigIn;
    friend class Wiper_IntIn;
    friend class Wiper_MixIn;

public:
    Wiper() : pMode(&digInMode),
              digInMode(*this),
              intInMode(*this),
              mixInMode(*this) {}

    void SetConfig(Config_Wiper *config)
    {
        pConfig = config;
        pParkSw = pVarMap[config->nParkInput];
        pSlowInput = pVarMap[config->nSlowInput];
        pFastInput = pVarMap[config->nFastInput];
        pInterInput = pVarMap[config->nInterInput];
        pOnSw = pVarMap[config->nOnInput];
        pSpeedInput = pVarMap[config->nSpeedInput];
        pSwipeInput = pVarMap[config->nSwipeInput];
        pWashInput = pVarMap[config->nWashInput];

        switch (config->eMode)
        {
        case WiperMode::DigIn:
            pMode = &digInMode;
            break;

        case WiperMode::IntIn:
            pMode = &intInMode;
            break;

        case WiperMode::MixIn:
            pMode = &mixInMode;
            break;
        }
    };

    static const uint16_t nBaseIndex = 0x1900;

    void Update();
    WiperSpeed GetSpeed() { return eSelectedSpeed; }
    WiperState GetState() { return eState; }

    float fSlowOut;
    float fFastOut;
    float fParkOut;
    float fInterOut;
    float fWashOut;
    float fSwipeOut;

private:
    void Parking();
    void Parked();
    void Slow();
    void Fast();
    void InterOn();
    void InterPause();
    void Wash();
    void Swipe();

    MotorSpeed GetMotorSpeed();
    void SetMotorSpeed(MotorSpeed speed);

    void UpdateInter();
    void CheckWash();
    void CheckSwipe();

    bool GetParkSw();
    bool GetOnSw() { return *pOnSw; };
    bool GetInterInput() { return *pInterInput; };
    bool GetSlowInput() { return *pSlowInput; };
    bool GetFastInput() { return *pFastInput; };
    bool GetWashInput() { return *pWashInput; };
    bool GetSwipeInput() { return *pSwipeInput; };
    uint16_t GetSpeedInput() { return (*pSpeedInput < NUM_WIPER_SPEED_MAP) ? *pSpeedInput : 0; };

    Wiper_Mode *pMode;
    Wiper_DigIn digInMode;
    Wiper_IntIn intInMode;
    Wiper_MixIn mixInMode;

    Config_Wiper *pConfig;

    WiperState eState;

    float *pParkSw;
    float *pSwipeInput;
    float *pWashInput;

    uint16_t nInterDelay;
    uint8_t nWashWipeCount;

    // DigIn Mode
    float *pSlowInput;
    float *pFastInput;
    float *pInterInput;

    // IntIn Mode
    float *pSpeedInput;
    WiperSpeed eSelectedSpeed;

    // MixIn Mode
    float *pOnSw;

    // Internal
    MotorSpeed eLastMotorSpeed;
    uint32_t nMotorOnTime;
    uint32_t nInterPauseStartTime;
    WiperState eLastState;
    uint16_t nLastParkSw;
    WiperState eStatePreWash;
};
