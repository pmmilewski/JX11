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
    float panLeft, panRight;

    void reset()
    {
        osc1.reset();
        osc2.reset();
        note = 0;
        saw = 0.0f;
        env.reset();
        panLeft = panRight = 0.707f;
    }

    void release()
    {
        env.release();
    }

    void updatePanning()
    {
        float panning = std::clamp((note - 60.0f) / 24.0f, -1.0f, 1.0f);
        panLeft = std::sin(PI_OVER_4 * (1.0f - panning));
        panRight = std::sin(PI_OVER_4 * (1.0f + panning));
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
