#pragma once

constexpr float SILENCE = 0.0001f; // -80 dB

class Envelope
{
public:
    float nextValue()
    {
        level *= multiplier;
        return level;
    }

    float level;
    float multiplier;
};