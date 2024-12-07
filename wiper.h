#pragma once

#include <cstdint>
#include "enums.h"
#include "config.h"

class Wiper
{
public:
    Wiper() {

    };

    void Update(uint32_t timeNow);
    WiperMode GetMode() { return pConfig->eMode; }
    WiperSpeed GetSpeed() { return eSelectedSpeed; }
    WiperState GetState() { return eState; }

    void SetConfig(Config_Wiper *config, uint16_t *pVarMap[PDM_VAR_MAP_SIZE])
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
    };

    uint16_t nSlowOut;
    uint16_t nFastOut;

private:
    void Parked();
    void Parking();
    void Slow();
    void Fast();
    void InterPause(uint32_t nTimeNow);
    void InterOn(uint32_t nTimeNow);
    void Wash();
    void Swipe();

    void UpdateInter();

    void SetMotorSpeed(MotorSpeed speed);
    void ReqState(WiperState state);

    void ModeUpdate();
    void DigInUpdate();
    void IntInUpdate();
    void MixInUpdate();

    Config_Wiper *pConfig;

    WiperState eState;
    WiperState eNextState;

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
    uint32_t nInterPauseStartTime;
    WiperState eLastState;
    uint16_t nLastParkSw;
    WiperState eStatePreWash;
};