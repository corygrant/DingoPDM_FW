#pragma once

#include "wiper.h"

class Wiper_MixIn : public IWiperMode{
    public:
        void Update(Wiper& wiper, uint32_t nTimeNow) override;
};