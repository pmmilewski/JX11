#pragma once
#include "Oscillator.h"

struct Voice
{
    int note;
    Oscillator osc;

    void reset()
    {
        note = -1;
    }

    float render()
    {
        return osc.nextSample();
    }
};
