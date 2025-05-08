#pragma once
#include <cmath>

const float TWO_PI = 6.2831853071795864f;

class Oscillator
{
public:
    float amplitude;
    float inc;
    float phase;

    void reset()
    {
        phase = 1.5707963268f;
    }

    float nextSample()
    {
        phase += inc;
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
        }

        return  amplitude * std::sin(TWO_PI * phase);;
    }

};

//sine osc based on digital resonator concept
#if 0
class Oscillator
{
public:
    float amplitude;
    float inc;
    float phase;

    float sin0;
    float sin1;
    float dsin;

    void reset()
    {
        phase = 0;

        sin0 = amplitude * std::sin(phase * TWO_PI);
        sin1 = amplitude * std::sin((phase - inc) * TWO_PI);
        dsin = 2.0f * std::cos(inc * TWO_PI);
    }

    float nextSample()
    {
        float sinx = dsin * sin0 - sin1;
        sin1 = sin0;
        sin0 = sinx;
        return sinx;
    }
};
#endif