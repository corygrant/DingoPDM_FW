#pragma once

#include "wiper.h"

class Wiper_IntIn : public IWiperMode{
    public:
        void Update(Wiper& wiper, uint32_t nTimeNow) override;
};