#pragma once
#include "DistrhoPluginInfo.h"
#include <cstdint>

__attribute__((unused)) static constexpr float TONE_PITCH_MAX_HZ = 16000.0;
__attribute__((unused)) static constexpr float NOISE_PITCH_MAX_HZ = 64000.0;

__attribute__((unused)) static constexpr float DECAY_TIME_MAX_MS = 500;
__attribute__((unused)) static constexpr float RELEASE_TIME_MAX_MS = 1000;
__attribute__((unused)) static constexpr float NOISE_BURST_MAX_MS = 250;

__attribute__((unused)) static constexpr unsigned SYNTH_NOTES = 8;
__attribute__((unused)) static constexpr unsigned SYNTH_CHANNELS = 9;

__attribute__((unused)) static constexpr unsigned OVERSAMPLING = 8;

__attribute__((unused)) static constexpr unsigned MAX_NAME_LEN = 32;

__attribute__((unused)) static constexpr float OVERDRIVE_MAX = 10.0;

__attribute__((unused)) static constexpr float RETRIGGER_MAX_MS = 50;
__attribute__((unused)) static constexpr unsigned RETRIGGER_MAX_COUNT = 4;

__attribute__((unused)) static constexpr float FILTER_CUTOFF_MIN_HZ = 10;
__attribute__((unused)) static constexpr float FILTER_CUTOFF_MAX_HZ = 8000;

__attribute__((unused)) static constexpr float MIN_UPDATE_RATE = 10.0;
__attribute__((unused)) static constexpr float MAX_UPDATE_RATE = (250.0-MIN_UPDATE_RATE);

__attribute__((unused)) static const char *const pNoteNames[SYNTH_NOTES] = {
    "C (BD - Bass Drum)",
    "C# (CC - Crash Cymbal)",
    "D (SD - Snare Drum)",
    "D# (RD - Ride)",
    "E (CP - Clap)",
    "F (CW - Cowbell)",
    "F#,G#,A# (HH - Hi-Hat)",
    "G,A,B (TM - Tom)"
};

__attribute__((unused)) static const char *const pWaveformNames[4] = {
    "Square",
    "Saw",
    "Triangle",
    "Sine"
};

__attribute__((unused)) static const char *const pRetrigRouteNames[3] = {
    "Both",
    "Tone only",
    "Noise only"
};



__attribute__((unused)) static const char *const pFilterRouteNames[4] = {
    "Both",
    "Tone only",
    "Noise only",
    "None"
};

__attribute__((unused)) static const float pBitDepth[] = {2.0f,4.0f,8.0f,16.0f,32.0f,64.0f,256.0f,0};
__attribute__((unused)) static const char *const pBitDepthNames[] = {"1 bit","2 bit","3 bit","4 bit","5 bit","6 bit","8 bit","Analog"};

inline int ParameterNoteNumber(int p)
{
    if (p < Parameter_NoteLevelFirstGroup || (unsigned)p >= Parameter_NoteLevelLastGroup + SYNTH_NOTES)
        return -1;
    return (p - Parameter_NoteLevelFirstGroup) % SYNTH_NOTES;
}

inline int ParameterFirstOfGroup(int p)
{
    int n = ParameterNoteNumber(p);
    return (n == -1) ? p : (p - n);
}

void InitNoise(uint8_t noise[65536]);

float FloatToHz(float value, float range);
int FloatToNoisePeriod(float value);

float OverdriveValue(float value);

float SynthGetSample(int32_t wave, float acc, float over);

std::string GetParameterDisplay(uint32_t index, double value);
