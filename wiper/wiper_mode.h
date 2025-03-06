#pragma once


class Wiper;  // Forward declaration

class Wiper_Mode
{
public:
    Wiper_Mode(Wiper& w) : wiper(w) {}
    virtual void Update() = 0;
protected:
    Wiper& wiper;
};