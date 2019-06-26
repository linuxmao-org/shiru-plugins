#pragma once
#include "DistrhoPluginInfo.h"

__attribute__((unused)) static constexpr int NOISE_MAX_BURSTS = 8;
__attribute__((unused)) static constexpr int NOISE_MAX_DURATION = 1000;
__attribute__((unused)) static constexpr int NOISE_MAX_PITCH = 32000.0f;

__attribute__((unused)) static constexpr int SYNTH_NOTES = 12;
__attribute__((unused)) static constexpr int SYNTH_CHANNELS = 8;

#define OVERSAMPLING		8

__attribute__((unused)) static constexpr unsigned MAX_NAME_LEN = 32;

__attribute__((unused)) static const char *const NoteNames[12] = {
    "C (Kick)",
    "C# (Crash)",
    "D (Snare)",
    "D# (Ride)",
    "E (Clap)",
    "F (Cowbell)",
    "F# (Closed hat)",
    "G (Low tom)",
    "G# (Medium hat)",
    "A (Mid tom)",
    "A# (Open hat)",
    "B (Hi tom)"
};

inline int ParameterNoteNumber(int p)
{
    int first = pIdNoteLevelFirstGroup;
    int end = first + (pIdDrumVolume2 - pIdDrumVolume1) * SYNTH_NOTES;
    if (p < first || p >= end)
        return -1;
    return (p - first) / (pIdDrumVolume2 - pIdDrumVolume1);
}

inline int ParameterFirstOfNoteGroup(int p)
{
    int n = ParameterNoteNumber(p);
    return (n == -1) ? p : (p - n * (pIdDrumVolume2 - pIdDrumVolume1));
}

inline int ParameterBurstNumber(int p)
{
    p = ParameterFirstOfNoteGroup(p);
    if (p == -1 || p < pIdBurstLevelFirstGroup)
        return -1;
    int n = (p - pIdBurst1Duration1) / (pIdBurst2Duration1 - pIdBurst1Duration1);
    return (n < NOISE_MAX_BURSTS) ? n : -1;
}

inline int ParameterFirstOfBurstGroup(int p)
{
    int n = ParameterBurstNumber(p);
    return (n == -1) ? p : (p - n * (pIdBurst2Duration1 - pIdBurst1Duration1));
}

inline int ParameterFirstOfGroup(int p)
{
    p = ParameterFirstOfNoteGroup(p);
    p = ParameterFirstOfBurstGroup(p);
    return p;
}
