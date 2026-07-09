#include "wiper_intin.h"
#include "wiper.h"

void Wiper_IntIn::CheckInputs()
{
    // Map speed input to selected speed
    wiper.eSelectedSpeed = wiper.pConfig->eSpeedMap[wiper.GetSpeedInput()];

    if (wiper.eState == WiperState::Swipe)
        return;

    if ((wiper.eSelectedSpeed >= WiperSpeed::Intermittent1) &&
        (wiper.eSelectedSpeed <= WiperSpeed::Intermittent6) &&  
        (wiper.eState != WiperState::IntermittentOn) &&
        (wiper.eState != WiperState::IntermittentPause))
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::IntermittentOn;
        return;
    }

    if (wiper.eSelectedSpeed == WiperSpeed::Slow)
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::Slow;
        return;
    }

    if (wiper.eSelectedSpeed == WiperSpeed::Fast)
    {
        wiper.SetMotorSpeed(MotorSpeed::Fast);
        wiper.eState = WiperState::Fast;
        return;
    }

    if ((wiper.eSelectedSpeed == WiperSpeed::Park) &&
        (wiper.eState != WiperState::Parked) &&
        (wiper.eState != WiperState::Wash))
    {
        wiper.eState = WiperState::Parking;
        return;
    }
}