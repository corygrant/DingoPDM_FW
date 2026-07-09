#include "wiper_digin.h"
#include "wiper.h"

void Wiper_DigIn::CheckInputs()
{
    if (wiper.eState == WiperState::Swipe)
        return;

    if ((wiper.GetInterInput() &&
         (wiper.eState != WiperState::IntermittentOn) &&
         (wiper.eState != WiperState::IntermittentPause)))
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::IntermittentOn;
        wiper.eSelectedSpeed = WiperSpeed::Intermittent1;
        return;
    }

    if (wiper.GetSlowInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::Slow;
        wiper.eSelectedSpeed = WiperSpeed::Slow;
        return;
    }

    if (wiper.GetFastInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Fast);
        wiper.eState = WiperState::Fast;
        wiper.eSelectedSpeed = WiperSpeed::Fast;
        return;
    }

    if ((wiper.eState == WiperState::Slow) && (!wiper.GetSlowInput()))
    {
        wiper.eState = WiperState::Parking;
        return;
    }

    if ((wiper.eState == WiperState::Fast) && (!wiper.GetFastInput()))
    {
        wiper.eState = WiperState::Parking;
        return;
    }

    if ((wiper.eState == WiperState::IntermittentOn) && (!wiper.GetInterInput()))
    {
        wiper.eState = WiperState::Parking;
        return;
    }

    if ((wiper.eState == WiperState::IntermittentPause) && (!wiper.GetInterInput()))
    {
        wiper.eState = WiperState::Parking;
        return;
    }
}