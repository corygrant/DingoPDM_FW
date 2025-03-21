#include "wiper_intin.h"
#include "wiper.h"

void Wiper_MixIn::CheckInputs()
{
    if (wiper.eState == WiperState::Swipe)
        return;

    if (!wiper.GetOnSw())
    {
        if (wiper.eState != WiperState::Parked)
        {
            wiper.eState = WiperState::Parking;
        }
        
        return;
    }

    //Same logic as IntIn mode
    Wiper_IntIn::CheckInputs();
}