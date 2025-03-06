#include "wiper_digin.h"
#include "wiper.h"

void Wiper_DigIn::Update(Wiper& wiper, uint32_t nTimeNow)
{
    wiper.SetSpeed(wiper.pConfig->eSpeedMap[*wiper.pSpeedInput]);

    switch (wiper.eSelectedSpeed)
    {
    case WiperSpeed::Park:
        wiper.ReqState(WiperState::Parking);
        break;

    case WiperSpeed::Slow:
        wiper.ReqState(WiperState::Slow);
        break;

    case WiperSpeed::Fast:
        wiper.ReqState(WiperState::Fast);
        break;

    case WiperSpeed::Intermittent1... WiperSpeed::Intermittent6:
        wiper.UpdateInter();
        wiper.ReqState(WiperState::IntermittentOn);
        break;
    }
}