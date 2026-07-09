#pragma once

#include "wiper_mode.h"

class Wiper_DigIn : public Wiper_Mode
{
public:
    Wiper_DigIn(Wiper &w) : Wiper_Mode(w) {}
    void CheckInputs() override;
};