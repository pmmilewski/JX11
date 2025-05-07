#pragma once

class NoiseGenerator
{
public:
    void reset()
    {
        noiseSeed = 22222;
    }

    float nextValue()
    {
        noiseSeed = noiseSeed * 196314165 + 907633515;
        const int temp = static_cast<int>(noiseSeed >> 7) - 16777216;

        return static_cast<float>(temp) / 16777216.0f;
    }

private:
    unsigned int noiseSeed;
};
