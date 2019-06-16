#pragma once
#include "DistrhoPlugin.hpp"
#include <array>
#include <string>
#include <cstdint>
namespace DISTRHO { struct Parameter; }

struct SynthOscObject {
    float acc;
    float add;
    float cut;

    int32_t noise;
    int32_t noise_seed;
};

extern const std::array<int32_t, 65536> Noise;

float OverdriveValue(float value);
float SynthGetSample(SynthOscObject *osc, float over, float duty, int wave);

float SynthEnvelopeTimeToDelta(float value, float max_ms);

float FloatToMultiple(float value);

void InitParameter(uint32_t index, Parameter &parameter);
std::string GetParameterDisplay(uint32_t index, double value);

__attribute__((unused)) static constexpr float OSC_CUT_MAX_MS = 1000;

__attribute__((unused)) static constexpr float MOD_DELAY_MAX_MS = 10000;
__attribute__((unused)) static constexpr float LFO_MAX_HZ = 50;

__attribute__((unused)) static constexpr float DETUNE_SEMITONES = 7.0;

__attribute__((unused)) static constexpr float FILTER_CUTOFF_MIN_HZ = 10;
__attribute__((unused)) static constexpr float FILTER_CUTOFF_MAX_HZ = 22050;
__attribute__((unused)) static constexpr float FILTER_MIN_RESONANCE = 1.0;
__attribute__((unused)) static constexpr float FILTER_MAX_RESONANCE = 10.0;

__attribute__((unused)) static constexpr float ENVELOPE_UPDATE_RATE_HZ = 500;

__attribute__((unused)) static constexpr float ENVELOPE_ATTACK_MAX_MS = 10000;
__attribute__((unused)) static constexpr float ENVELOPE_DECAY_MAX_MS = 10000;
__attribute__((unused)) static constexpr float ENVELOPE_RELEASE_MAX_MS = 1000;

__attribute__((unused)) static constexpr float OVERDRIVE_MAX = 10.0;

__attribute__((unused)) static const char *const OscWaveformNames[] = {
    "Square/Pulse",
    "Tri/Saw",
    "Sine",
    "Digi Noise"
};

__attribute__((unused)) static const char *const SlideRouteNames[3] = {
    "Both",
    "OscA only",
    "OscB only"
};
