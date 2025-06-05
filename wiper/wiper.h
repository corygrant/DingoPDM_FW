#pragma once

#include <cstdint>
#include "enums.h"
#include "port.h"
#include "config.h"
#include "wiper_mode.h"
#include "wiper_digin.h"
#include "wiper_intin.h"
#include "wiper_mixin.h"

extern uint16_t *pVarMap[PDM_VAR_MAP_SIZE];

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

    void Update();
    WiperSpeed GetSpeed() { return eSelectedSpeed; }
    WiperState GetState() { return eState; }
    static MsgCmdResult ProcessSettingsMsg(PdmConfig *conf, CANRxFrame *rx, CANTxFrame *tx);

    uint16_t nSlowOut;
    uint16_t nFastOut;
    uint16_t nParkOut;
    uint16_t nInterOut;
    uint16_t nWashOut;
    uint16_t nSwipeOut;

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
    uint16_t GetSpeedInput() { return (*pSpeedInput < PDM_NUM_WIPER_SPEED_MAP) ? *pSpeedInput : 0; };

    Wiper_Mode *pMode;
    Wiper_DigIn digInMode;
    Wiper_IntIn intInMode;
    Wiper_MixIn mixInMode;

    Config_Wiper *pConfig;

    WiperState eState;

    uint16_t *pParkSw;
    uint16_t *pSwipeInput;
    uint16_t *pWashInput;

    uint16_t nInterDelay;
    uint8_t nWashWipeCount;

    // DigIn Mode
    uint16_t *pSlowInput;
    uint16_t *pFastInput;
    uint16_t *pInterInput;

    // IntIn Mode
    uint16_t *pSpeedInput;
    WiperSpeed eSelectedSpeed;

    // MixIn Mode
    uint16_t *pOnSw;

    // Internal
    MotorSpeed eLastMotorSpeed;
    uint32_t nMotorOnTime;
    uint32_t nInterPauseStartTime;
    WiperState eLastState;
    uint16_t nLastParkSw;
    WiperState eStatePreWash;
};
