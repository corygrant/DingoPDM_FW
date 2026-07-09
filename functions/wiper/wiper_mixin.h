#pragma once

#include "wiper_intin.h"

class Wiper_MixIn : public Wiper_IntIn
{
public:
    Wiper_MixIn(Wiper &w) : Wiper_IntIn(w) {}
    void CheckInputs() override;
};