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
    float target;
    float glideRate;
    float panning;
    float targetPanning;
    float panLeft, panRight;

    void reset()
    {
        osc1.reset();
        osc2.reset();
        note = 0;
        saw = 0.0f;
        env.reset();
        panLeft = panRight = 0.707f;
        panning = targetPanning = 0.0f;
    }

    void release()
    {
        env.release();
    }

    void updatePanning()
    {
        panning += glideRate * (targetPanning - panning);
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

    void updateLFO()
    {
        period += glideRate * (target - period);
        updatePanning();
    }

};
