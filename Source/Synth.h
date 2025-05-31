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
    juce::LinearSmoothedValue<float> outputLevelSmoother;

    float velocitySensitivity;
    bool ignoreVelocity;

    float envAttack;
    float envDecay;
    float envSustain;
    float envRelease;

    const int LFO_MAX = 32;
    float lfoInc;
    float vibrato;
    float pwmDepth;
    int lfoWave;

    int glideMode;
    float glideRate;
    float glideBend;

    static constexpr int MAX_VOICES = 8;
    int numVoices;
    int prevNumVoices;
    float filterKeyTracking;
    float filterQ;
    float resonanceCtl;
    float filterLFODepth;


private:
    float sampleRate;
    float pitchBend;
    bool sustainPedalPressed;
    std::array<Voice, MAX_VOICES> voices;
    NoiseGenerator noiseGenerator;
    int lfoStep;
    float lfo;
    float modWheel;
    int lastNote;

    float calcPeriod(int v, int note) const;
    void startVoice(int v, int note, int velocity);
    void restartMonoVoice(int note, int velocity);
    void noteOn(int note, int velocity);
    void noteOff(int note);
    int findFreeVoice() const;
    void shiftQueuedNotes();
    int nextQueuedNote();
    void updateLFO();

    void updatePeriod(Voice& voice) const
    {
        voice.osc1.period = voice.period * pitchBend;
        voice.osc2.period = voice.osc1.period * detune;
    }

    bool isPlyingLegatoStyle() const;
};
