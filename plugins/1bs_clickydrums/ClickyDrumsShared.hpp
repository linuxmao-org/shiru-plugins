#pragma once
#include "DistrhoPluginInfo.h"

__attribute__((unused)) static constexpr int SYNTH_NOTES = 12;
__attribute__((unused)) static constexpr int SYNTH_CHANNELS = 8;

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
    int end = first + (pIdDuration2 - pIdDuration1) * SYNTH_NOTES;
    if (p < first || p >= end)
        return -1;
    return (p - first) / (pIdDuration2 - pIdDuration1);
}

inline int ParameterFirstOfGroup(int p)
{
    int n = ParameterNoteNumber(p);
    return (n == -1) ? p : (p - n * (pIdDuration2 - pIdDuration1));
}
