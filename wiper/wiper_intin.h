#pragma once

#include "wiper_mode.h"

class Wiper_IntIn : public Wiper_Mode
{
public:
    Wiper_IntIn(Wiper& w) : Wiper_Mode(w) {}
    void Update() override;

private:
    void Parked();
    void Slow();
    void Fast();
    void InterPause();
    void InterOn();
};