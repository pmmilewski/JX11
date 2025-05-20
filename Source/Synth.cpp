#include "Synth.h"
#include "ProtectYourEars.h"

Synth::Synth()
{
    sampleRate = 44100.0f;
}

void Synth::allocateResources(double sampleRate_, [[maybe_unused]] int samplesPerBlock_)
{
    sampleRate = static_cast<float>(sampleRate_);
}

void Synth::deallocateResources()
{
}

void Synth::reset()
{
    voice.reset();
    noiseGenerator.reset();
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    voice.osc1.period = voice.period;
    voice.osc2.period = voice.osc1.period * detune;

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        float noise = noiseGenerator.nextValue() * noiseMix;

        float output = 0.0f;
        if (voice.note != -1)
        {
            output = voice.render(noise);
        }

        outputBufferLeft[sample] = output;
        if (outputBufferRight != nullptr)
        {
            outputBufferRight[sample] = output;
        }
    }

    if (!voice.env.isActive())
    {
        voice.env.reset();
    }
}

void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0)
    {
    case 0x80:
        noteOff(data1 & 0x7F);
        break;
    case 0x90:
        uint8_t note = data1 & 0x7F;
        uint8_t velocity = data2 & 0x7F;
        if (velocity > 0)
        {
            noteOn(note, velocity);
        }
        else
        {
            noteOff(note);
        }
        break;
    }
}

float Synth::calcPeriod(int note) const
{
    float period = tune * std::exp(-0.05776226505f * static_cast<float>(note));
    return period;
}

void Synth::noteOn(int note, int velocity)
{
    voice.note = note;

    float period = calcPeriod(note);
    voice.period = period;

    voice.osc1.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;

    Envelope& env = voice.env;

    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::noteOff(int note)
{
    if (voice.note == note)
    {
        voice.release();
    }
}
