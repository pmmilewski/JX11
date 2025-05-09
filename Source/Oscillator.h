#pragma once

#include <cmath>

constexpr float TWO_PI = 6.2831853071795864f;
constexpr float PI = 3.1415926535897932f;
constexpr float PI_OVER_4 = 0.7853981633974483f;

class Oscillator
{
public:
    float period = 0.0f;
    float amplitude = 1.0f;

    void reset()
    {
        inc = 0.0f;
        phase = 0.0f;
    }

    float nextSample()
    {
        float output = 0.0f;

        phase += inc;

        if (phase <= PI_OVER_4)
        {
            float halfPeriod = period / 2.0f;
            phaseMax = std::floor(0.5f + halfPeriod) - 0.5f;
            phaseMax *= PI;

            inc = phaseMax / halfPeriod;
            phase = -phase;

            if (phase*phase > 1e-9)
            {
                output = amplitude * std::sin(phase) / phase;
            }
            else
            {
                output = amplitude;
            }
        }
        else
        {
            if (phase > phaseMax)
            {
                phase = phaseMax + phaseMax - phase;
                inc = -inc;
            }

            output = amplitude * std::sin(phase) / phase;
        }

        return output;
    }

private:
    float phase = 0;
    float phaseMax = 0;
    float inc = 0;
};
