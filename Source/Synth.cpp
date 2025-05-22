#include "Synth.h"
#include "ProtectYourEars.h"

static constexpr float ANALOG = 0.002f;
static constexpr int SUSTAIN = -1;
static constexpr float TWO_OVER_PI = 0.6366197723675813f;
static constexpr float PI_OVER_TWO = 1.5707963267948966f;

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
    sustainPedalPressed = false;
    outputLevelSmoother.reset(sampleRate, 0.05);
    lfo = 0.0f;
    lfoStep = 0;
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
        updateLFO();

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

        float outputLevel = outputLevelSmoother.getNextValue();
        outputLeft *= outputLevel;
        outputRight *= outputLevel;

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
    case 0xB0:
        {
            controlChange(data1, data2);
            break;
        }
    }
}

void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    switch (data1)
    {
    case 0x40:
        {
            sustainPedalPressed = (data2 >= 64);

            if (!sustainPedalPressed)
            {
                noteOff(SUSTAIN);
            }
            break;
        }
    default:
        {
            if (data1 >= 0x78)
            {
                for (int v =0; v < MAX_VOICES; ++v)
                {
                    voices[v].reset();
                }
            }
            sustainPedalPressed = false;
        }
    }
}

void Synth::releaseVoices()
{
    for (int v = 0; v < numVoices; ++v)
    {
        voices[v].reset();
        voices[v].note = 0;
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


    float vel = 0.004f * static_cast<float>((velocity + 64) * (velocity + 64)) - 8.0f;
    voice.osc1.amplitude = volumeTrim * vel;
    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;

    if (vibrato == 0.0f && pwmDepth > 0.0f) {
        voice.osc2.squareWave(voice.osc1, voice.period);
    }

    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::restartMonoVoice(int note, int velocity)
{
    float period = calcPeriod(0, note);
    Voice& voice = voices[0];
    voice.period = period;

    voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.updatePanning();
}

void Synth::noteOn(int note, int velocity)
{

    if (ignoreVelocity) { velocity = 80; }

    int v = 0; // mono

    if (numVoices == 1) // poly
    {
       if (voices[0].note > 0) // legato-style plying
       {
           shiftQueuedNotes();
           restartMonoVoice(note, velocity);
           return;
       }
    }
    else
    {
        v = findFreeVoice();
    }

    startVoice(v, note, velocity);
}

void Synth::noteOff(int note)
{
    if ((numVoices == 1)) {
        if (voices[0].note == note)
        {
            int queuedNote = nextQueuedNote();
            if (queuedNote > 0) {
                restartMonoVoice(queuedNote, -1);
            }
        }
        else // in case key further in queue was released
        {
            for (int v = 0; v < MAX_VOICES; ++v)
            {
                if (voices[v].note == note)
                {
                    voices[v].note = 0;
                    break;
                }
            }
        }
    }

    for (int v = 0; v < numVoices; ++v)
    {
        if (voices[v].note == note)
        {
            if (sustainPedalPressed)
            {
                voices[v].note = SUSTAIN;
            }
            else
            {
                voices[v].release();
                voices[v].note = 0;
            }
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

void Synth::shiftQueuedNotes()
{
    for (int tmp = MAX_VOICES - 1; tmp > 0; tmp--)
    {
        voices[tmp].note = voices[tmp - 1].note;
        voices[tmp].release();
    }
}

int Synth::nextQueuedNote()
{
    int held = 0;
    for (int v = MAX_VOICES - 1; v > 0; v--)
    {
        if (voices[v].note > 0) { held = v; }
    }

    if (held > 0)
    {
        int note = voices[held].note;
        voices[held].note = 0;
        return note;
    }

    return 0;
}

void Synth::updateLFO()
{
    if (--lfoStep <= 0)
    {
        lfoStep = LFO_MAX;

        lfo += lfoInc;
        if (lfo > PI) { lfo -= TWO_PI; }

        float vibratoMod = 0.0f, pwm = 0.0f;

        if (lfoWave == 0 || vibrato <= 0)
        {
            const float sine = std::sin(lfo);
            vibratoMod = 1.0f + sine * vibrato;
            pwm = 1.0f + sine * pwmDepth;
        }
        else if (lfoWave == 1)
        {
            float triangle = 0.0f;
            if (std::abs(lfo) < PI_OVER_TWO)
            {
                triangle = lfo * TWO_OVER_PI;
            }
            else
            {
                if (lfo > 0.0f)
                {
                    triangle = -lfo * TWO_OVER_PI + 2.0f;
                }
                else
                {
                    triangle = -lfo * TWO_OVER_PI - 2.0f;
                }
            }

            vibratoMod = 1.0f + triangle * vibrato;
            pwm = 1.0f + triangle * pwmDepth;
        }
        else if (lfoWave == 2)
        {
            float saw = 0.0f;
            if (std::abs(lfo) < PI_OVER_TWO)
            {
                saw = lfo * TWO_OVER_PI;
            }
            else
            {
                if (lfo > 0.0f)
                {
                    saw = lfo * TWO_OVER_PI - 2.0f;
                }
                else
                {
                    saw = lfo * TWO_OVER_PI + 2.0f;
                }
            }

            vibratoMod = 1.0f + saw * vibrato;
            pwm = 1.0f + saw * pwmDepth;
        }
        else if (lfoWave == 3)
        {
            if (lfo >= 0.0f)
            {
                vibratoMod = 1.0f + vibrato;
                pwm = 1.0f + pwmDepth;
            }
            else
            {
                vibratoMod = 1.0f - vibrato;
                pwm = 1.0f - pwmDepth;
            }
        }

        for (int v = 0; v < numVoices; ++v)
        {
            Voice& voice = voices[v];
            if (voice.env.isActive())
            {
                voice.osc1.modulation = vibratoMod;
                voice.osc2.modulation = pwm;
            }
        }
    }
}
