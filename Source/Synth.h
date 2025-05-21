#pragma once

#include <JuceHeader.h>
#include "Voice.h"
#include "NoiseGenerator.h"

class Synth
{
public:
    Synth();

    void allocateResources(double sampleRate, int samplesPerBlock);
    void deallocateResources();
    void reset();
    void render(float** outputBuffers, int sampleCount);
    void midiMessage(uint8_t data0, uint8_t data1, uint8_t data2);
    void controlChange(uint8_t data1, uint8_t data2);
    void releaseVoices();

    float noiseMix;
    float oscMix;
    float detune;
    float tune;
    float volumeTrim;
    float outputLevel;

    float envAttack;
    float envDecay;
    float envSustain;
    float envRelease;

    static constexpr int MAX_VOICES = 8;
    int numVoices;
    int prevNumVoices;

private:
    float sampleRate;
    float pitchBend;
    bool sustainPedalPressed;
    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGenerator;

    float calcPeriod(int v, int note) const;
    void startVoice(int v, int note, int velocity);
    void noteOn(int note, int velocity);
    void noteOff(int note);
    int findFreeVoice() const;
};
