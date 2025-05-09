#pragma once
#include "Oscillator.h"

struct Voice
{
    int note;
    Oscillator osc;
    float saw;

    void reset()
    {
        note = -1;
        saw = 0.0f;
    }

    float render()
    {
        float sample = osc.nextSample();
        saw = saw * 0.997f + sample;
        return saw;
    }
};
