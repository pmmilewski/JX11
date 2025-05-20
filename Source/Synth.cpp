#include "Synth.h"
#include "ProtectYourEars.h"

static constexpr float ANALOG = 0.002f;

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
    for (int v = 0; v < MAX_VOICES; ++v)
    {
        voices[v].reset();
    }

    noiseGenerator.reset();
    pitchBend = 1.0f;
}

void Synth::render(float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    for (int v = 0; v < numVoices; ++v)
    {
        if (Voice& voice = voices[v]; voice.env.isActive())
        {
            voice.osc1.period = voice.period * pitchBend;
            voice.osc2.period = voice.osc1.period * detune;
        }
    }

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        float noise = noiseGenerator.nextValue() * noiseMix;

        float outputLeft = 0.0f;
        float outputRight = 0.0f;

        for (int v = 0; v < numVoices; ++v)
        {
            if (Voice& voice = voices[v]; voice.env.isActive())
            {
                float output = voice.render(noise);
                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
            }
        }

        if (outputBufferRight != nullptr)
        {
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        }
        else
        {
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5f;
        }
    }

    for (int v = 0; v < numVoices; ++v)
    {
        if (Voice& voice = voices[v]; !voice.env.isActive())
        {
            voice.env.reset();
        }
    }
}

void Synth::midiMessage(uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0)
    {
    case 0x80:
        {
            noteOff(data1 & 0x7F);
            break;
        }
    case 0x90:
        {
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
    case 0xE0:
        {
            pitchBend = std::exp(-0.000014102f * static_cast<float>(data1 + 128 * data2 - 8192));
            break;
        }
    }
}

void Synth::releaseVoices()
{
    for (int v = 0; v < numVoices; ++v)
    {
        voices[v].reset();
        voices[v].note = -1;
    }
}

float Synth::calcPeriod(int v, int note) const
{
    float period = tune * std::exp(-0.05776226505f * (static_cast<float>(note) + ANALOG * static_cast<float>(v)));
    while (period < 6.0f || (period * detune) < 6.0f) { period += period; } // BLIT osc may now work properly with such small period
    return period;
}

void Synth::startVoice(int v, int note, int velocity)
{
    // earlier simple formula was used
    // float freq = 440.0f * std::exp2((float(note - 69) + tune) / 12.0f);
    float period = calcPeriod(v, note);

    Voice& voice = voices[v];
    voice.period = period;
    voice.note = note;
    voice.updatePanning();


    voice.osc1.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;

    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::noteOn(int note, int velocity)
{
    int v = 0; // mono

    if (numVoices > 1) // poly
    {
        v = findFreeVoice();
    }

    startVoice(v, note, velocity);
}

void Synth::noteOff(int note)
{
    for (int v = 0; v < numVoices; ++v)
    {
        if (voices[v].note == note)
        {
            voices[v].release();
            voices[v].note = -1;
        }
    }
}

int Synth::findFreeVoice() const
{
    int v = 0;
    float l = 100.0f; // louder than anything

    for (int i = 0; i < numVoices; ++i)
    {
        if (voices[i].env.level < l && !voices[i].env.isActive())
        {
            l = voices[i].env.level;
            v = i;
        }
    }

    return v;
}
