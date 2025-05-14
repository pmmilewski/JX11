#pragma once
#include "Oscillator.h"
#include "Envelope.h"

struct Voice
{
    int note;
    Oscillator osc;
    float saw;
    Envelope env;

    void reset()
    {
        note = -1;
        saw = 0.0f;
    }

    float render(float input)
    {
        float sample = osc.nextSample();
        saw = saw * 0.997f + sample;
        float output = saw + input;

        float envelope = env.nextValue();
        return output * envelope;
    }
};
