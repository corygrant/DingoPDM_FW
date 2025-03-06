#include "wiper_digin.h"
#include "wiper.h"  

void Wiper_MixIn::Update()
{
    switch (wiper.eState)
    {
    case WiperState::Parked:
        Parked();
        break;

    case WiperState::Parking:
        wiper.Parking();
        break;

    case WiperState::Slow:
        Slow();
        break;

    case WiperState::Fast:
        Fast();
        break;

    case WiperState::IntermittentOn:
        InterOn();
        break;

    case WiperState::IntermittentPause:
        InterPause();
        break;

    case WiperState::Wash:
        wiper.Wash();
        break;

    case WiperState::Swipe:
        wiper.Swipe();
        break;
    }

    wiper.CheckWash();
    wiper.CheckSwipe();
}

void Wiper_MixIn::Parked()
{
    wiper.SetMotorSpeed(MotorSpeed::Off);

    if (wiper.GetInterInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::IntermittentOn;
        wiper.eSelectedSpeed = WiperSpeed::Intermittent1;
    }

    if (wiper.GetSlowInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::Slow;
        wiper.eSelectedSpeed = WiperSpeed::Slow;
    }

    if (wiper.GetFastInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Fast);
        wiper.eState = WiperState::Fast;
        wiper.eSelectedSpeed = WiperSpeed::Fast;
    }
}

void Wiper_MixIn::Slow()
{
    wiper.eLastState = wiper.eState;

    wiper.SetMotorSpeed(MotorSpeed::Slow);

    if (!wiper.GetSlowInput())
        wiper.eState = WiperState::Parking;

    if (wiper.GetInterInput())
        wiper.eState = WiperState::IntermittentOn;

    if (wiper.GetFastInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Fast);
        wiper.eState = WiperState::Fast;
    }
}

void Wiper_MixIn::Fast()
{
    wiper.eLastState = wiper.eState;

    wiper.SetMotorSpeed(MotorSpeed::Fast);

    if (!wiper.GetFastInput())
        wiper.eState = WiperState::Parking;

    if (wiper.GetInterInput())
        wiper.eState = WiperState::IntermittentOn;

    if (wiper.GetSlowInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::Slow;
    }
}

void Wiper_MixIn::InterOn()
{
    wiper.eLastState = wiper.eState;

    wiper.SetMotorSpeed(MotorSpeed::Slow);

    if (!wiper.GetInterInput())
        wiper.eState = WiperState::Parking;

    if (wiper.GetSlowInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::Slow;
    }

    if (wiper.GetFastInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Fast);
        wiper.eState = WiperState::Fast;
    }

    // Park detected
    // Stop motor
    // Save time - pause for set time
    if (!wiper.GetParkSw())
    {
        wiper.SetMotorSpeed(MotorSpeed::Off);
        wiper.nInterPauseStartTime = SYS_TIME;
        wiper.eState = WiperState::IntermittentPause;
    }
}

void Wiper_MixIn::InterPause()
{
    wiper.eLastState = wiper.eState;

    wiper.SetMotorSpeed(MotorSpeed::Off);

    if (!wiper.GetInterInput())
    {
        wiper.eState = WiperState::Parking;
    }

    if (wiper.GetSlowInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::Slow;
    }

    if (wiper.GetFastInput())
    {
        wiper.SetMotorSpeed(MotorSpeed::Fast);
        wiper.eState = WiperState::Fast;
    }

    // Pause for inter delay
    if ((SYS_TIME - wiper.nInterPauseStartTime) > wiper.nInterDelay)
    {
        wiper.SetMotorSpeed(MotorSpeed::Slow);
        wiper.eState = WiperState::IntermittentOn;
    }
}
