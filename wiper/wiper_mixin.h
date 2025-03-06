#pragma once

#include "wiper_mode.h"

class Wiper_MixIn : public Wiper_Mode
{
public:
    Wiper_MixIn(Wiper& w) : Wiper_Mode(w) {}
    void Update() override;

private:
    void Parked();
    void Slow();
    void Fast();
    void InterPause();
    void InterOn();
};