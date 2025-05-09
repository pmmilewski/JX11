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

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        float noise = noiseGenerator.nextValue();

        float output = 0.0f;
        if (voice.note != -1)
        {
            output = voice.render();
        }

        outputBufferLeft[sample] = output;
        if (outputBufferRight != nullptr)
        {
            outputBufferRight[sample] = output;
        }
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

void Synth::noteOn(int note, int velocity)
{
    voice.note = note;

    float freq = 440.0f * std::exp2(static_cast<float>(note - 69) / 12.0f);

    voice.osc.amplitude = (velocity / 127.0f) * 0.5f;
    voice.osc.period = sampleRate / freq;
    voice.osc.reset();
}

void Synth::noteOff(int note)
{
    if (voice.note == note)
    {
        voice.note = -1;
    }
}
