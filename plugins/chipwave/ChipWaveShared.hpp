#pragma once
#include <string>
#include <cstdint>

struct SynthOscObject {
    float acc;
    float add;
    float cut;

    int32_t noise;
    int32_t noise_seed;
};

void InitNoise(int32_t noise[65536]);

float OverdriveValue(float value);
float SynthGetSample(SynthOscObject *osc, const int32_t *noise, float over, float duty, int wave);

float SynthEnvelopeTimeToDelta(float value, float max_ms);

float FloatToMultiple(float value);

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
