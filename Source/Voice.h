#pragma once
#include "Oscillator.h"
#include "Envelope.h"

struct Voice
{
    Oscillator osc1;
    Oscillator osc2;

    int note;
    float saw;
    Envelope env;
    float period;

    void reset()
    {
        osc1.reset();
        osc2.reset();
        note = -1;
        saw = 0.0f;
        env.reset();
    }

    void release()
    {
        env.release();
    }

    float render(float input)
    {
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();
        saw = saw * 0.997f + sample1 - sample2;

        float output = saw + input;
        float envelope = env.nextValue();
        return output * envelope;
    }
};
