#include "Parameters.h"
#include "Synth.h"

template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& apvts,
                          const juce::ParameterID& id, T& destination)
{
  destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
  jassert(destination);
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
  castParameter(apvts, ParameterID::oscMix, oscMixParam);
  castParameter(apvts, ParameterID::oscTune, oscTuneParam);
  castParameter(apvts, ParameterID::oscFine, oscFineParam);
  castParameter(apvts, ParameterID::glideMode, glideModeParam);
  castParameter(apvts, ParameterID::glideRate, glideRateParam);
  castParameter(apvts, ParameterID::glideBend, glideBendParam);
  castParameter(apvts, ParameterID::filterFreq, filterFreqParam);
  castParameter(apvts, ParameterID::filterReso, filterResoParam);
  castParameter(apvts, ParameterID::filterEnv, filterEnvParam);
  castParameter(apvts, ParameterID::filterLFO, filterLFOParam);
  castParameter(apvts, ParameterID::filterVelocity, filterVelocityParam);
  castParameter(apvts, ParameterID::filterAttack, filterAttackParam);
  castParameter(apvts, ParameterID::filterDecay, filterDecayParam);
  castParameter(apvts, ParameterID::filterSustain, filterSustainParam);
  castParameter(apvts, ParameterID::filterRelease, filterReleaseParam);
  castParameter(apvts, ParameterID::envAttack, envAttackParam);
  castParameter(apvts, ParameterID::envDecay, envDecayParam);
  castParameter(apvts, ParameterID::envSustain, envSustainParam);
  castParameter(apvts, ParameterID::envRelease, envReleaseParam);
  castParameter(apvts, ParameterID::lfoRate, lfoRateParam);
  castParameter(apvts, ParameterID::lfoWaveform, lfoWaveformParam);
  castParameter(apvts, ParameterID::vibrato, vibratoParam);
  castParameter(apvts, ParameterID::noise, noiseParam);
  castParameter(apvts, ParameterID::octave, octaveParam);
  castParameter(apvts, ParameterID::tuning, tuningParam);
  castParameter(apvts, ParameterID::outputLevel, outputLevelParam);
  castParameter(apvts, ParameterID::polyMode, polyModeParam);
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
  juce::AudioProcessorValueTreeState::ParameterLayout layout{};

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::polyMode,
    "Polyphony",
    juce::NormalisableRange<float>(1.0f, static_cast<float>(Synth::MAX_VOICES), 1.0f),
    1));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::oscTune,
    "Osc Tune",
    juce::NormalisableRange<float>(-24.0f, 24.0f, 1.0f),
    -12.0f,
    juce::AudioParameterFloatAttributes().withLabel("semi")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
  ParameterID::oscFine,
  "Osc Fine",
  juce::NormalisableRange<float>(-50.0f, 50.0f, 0.1f, 0.3f, true),
  0.0f,
  juce::AudioParameterFloatAttributes().withLabel("cent")));

  auto oscMixStringFromValue = [](float value, int)
  {
    char s[16] = { 0 };
    snprintf(s, 16, "%4.0f : %2.0f", 100.0 - 0.5f * value, 0.5f * value);
    return juce::String(s);
  };

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::oscMix,
    "Osc Mix",
    juce::NormalisableRange<float>(0.0f, 100.0f),
    0.0f,
    juce::AudioParameterFloatAttributes()
    .withLabel("%")
    .withStringFromValueFunction(oscMixStringFromValue)));

  layout.add(std::make_unique<juce::AudioParameterChoice>(
    ParameterID::glideMode,
    "Glide Mode",
    juce::StringArray{"Off", "Legato", "Always"},
    0));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::glideRate,
    "Glide Rate",
    juce::NormalisableRange<float>(0.0f, 100.f, 1.0f),
    35.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::glideBend,
    "Glide Bend",
    juce::NormalisableRange<float>(-36.0f, 36.0f, 0.01f, 0.4f, true),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("semi")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterFreq,
    "Filter Freq",
    juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f),
    100.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterReso,
    "Filter Reso",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    15.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterEnv,
    "Filter Env",
    juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
    50.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterLFO,
    "Filter LFO",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  auto filterVelocityStringFromValue = [](float value, int)
  {
    if (value < -90.0f)
      return juce::String("OFF");
    else
      return juce::String(value);
  };

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterVelocity,
    "Velocity",
    juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
    0.0f,
    juce::AudioParameterFloatAttributes()
    .withLabel("%")
    .withStringFromValueFunction(filterVelocityStringFromValue)));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterAttack,
    "Filter Attack",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterDecay,
    "Filter Decay",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    30.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterSustain,
    "Filter Sustain",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::filterRelease,
    "Filter Release",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    25.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::envAttack,
    "Env Attack",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::envDecay,
    "Env Decay",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    50.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::envSustain,
    "Env Sustain",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    100.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::envRelease,
    "Env Release",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    30.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  auto lfoRateStringFromValue = [](float value, int)
  {
    float lfoHz = std::exp(7.0f * value - 4.0f);
    return juce::String(lfoHz, 3);
  };

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::lfoRate,
    "LFO Rate",
    juce::NormalisableRange<float>(),
    0.81f,
    juce::AudioParameterFloatAttributes()
    .withLabel("Hz")
    .withStringFromValueFunction(lfoRateStringFromValue)));

  juce::StringArray waveforms = {
    "Sine",
    "Triangle",
    "Saw",
    "Square"
  };

  layout.add(std::make_unique<juce::AudioParameterChoice>(ParameterID::lfoWaveform, "LFO Wave", waveforms, 0));

  auto vibratoStringFromValue = [](float value, int)
  {
    if (value < 0.0f)
      return "PWM " + juce::String(-value, 1);
    else
      return juce::String(value, 1);
  };

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::vibrato,
    "Vibrato",
    juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
    0.0f,
    juce::AudioParameterFloatAttributes()
    .withLabel("%")
    .withStringFromValueFunction(vibratoStringFromValue)));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::noise,
    "Noise",
    juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("%")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::octave,
    "Octave",
    juce::NormalisableRange<float>(-2.0f, 2.0f, 1.0f),
    0.0f));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::tuning,
    "Tuning",
    juce::NormalisableRange<float>(-100.0f, 100.0f, 0.1f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("cent")));

  layout.add(std::make_unique<juce::AudioParameterFloat>(
    ParameterID::outputLevel,
    "Output Level",
    juce::NormalisableRange<float>(-24.0f, 6.0f, 0.1f),
    0.0f,
    juce::AudioParameterFloatAttributes().withLabel("dB")));

  return layout;
}
