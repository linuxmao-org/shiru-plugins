#pragma once
#include <string>
#include <cstdint>

#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "ChipDrum"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/chipdrum"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/chipdrum"
#define DISTRHO_PLUGIN_UNIQUE_ID       'c','d','r','u'
#define DISTRHO_PLUGIN_VERSION         0,41,0
#define DISTRHO_PLUGIN_LABEL           "ChipDrum"
#define DISTRHO_PLUGIN_LICENSE         "WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v0.41 03.06.19 [LinuxMAO]"
#define DISTRHO_PLUGIN_NUM_INPUTS      0
#define DISTRHO_PLUGIN_NUM_OUTPUTS     8
#define DISTRHO_PLUGIN_IS_SYNTH        1
#define DISTRHO_PLUGIN_HAS_UI          1
#define DISTRHO_PLUGIN_HAS_EMBED_UI    1
#define DISTRHO_PLUGIN_HAS_EXTERNAL_UI 0
#define DISTRHO_PLUGIN_IS_RT_SAFE      1
#define DISTRHO_PLUGIN_WANT_PROGRAMS   1
#define DISTRHO_PLUGIN_WANT_STATE      1
#define DISTRHO_PLUGIN_WANT_FULL_STATE 1
#define DISTRHO_PLUGIN_NUM_PROGRAMS    PresetData.size()

enum {
    #define PER_NOTE(x) x##1, x##2, x##3, x##4, x##5, x##6, x##7, x##8

    PER_NOTE(Parameter_ToneLevel),
    PER_NOTE(Parameter_ToneDecay),
    PER_NOTE(Parameter_ToneSustain),
    PER_NOTE(Parameter_ToneRelease),
    PER_NOTE(Parameter_TonePitch),
    PER_NOTE(Parameter_ToneSlide),
    PER_NOTE(Parameter_ToneWave),
    PER_NOTE(Parameter_ToneOver),

    PER_NOTE(Parameter_NoiseLevel),
    PER_NOTE(Parameter_NoiseDecay),
    PER_NOTE(Parameter_NoiseSustain),
    PER_NOTE(Parameter_NoiseRelease),
    PER_NOTE(Parameter_NoisePitch1),
    PER_NOTE(Parameter_NoisePitch2),
    PER_NOTE(Parameter_NoisePitch2Off),
    PER_NOTE(Parameter_NoisePitch2Len),
    PER_NOTE(Parameter_NoisePeriod),
    PER_NOTE(Parameter_NoiseSeed),
    PER_NOTE(Parameter_NoiseType),

    PER_NOTE(Parameter_RetrigTime),
    PER_NOTE(Parameter_RetrigCount),
    PER_NOTE(Parameter_RetrigRoute),

    PER_NOTE(Parameter_FilterLP),
    PER_NOTE(Parameter_FilterHP),
    PER_NOTE(Parameter_FilterRoute),

    PER_NOTE(Parameter_DrumGroup),
    PER_NOTE(Parameter_DrumBitDepth),
    PER_NOTE(Parameter_DrumUpdateRate),
    PER_NOTE(Parameter_DrumVolume),
    PER_NOTE(Parameter_DrumPan),

    PER_NOTE(Parameter_VelDrumVolume),
    PER_NOTE(Parameter_VelTonePitch),
    PER_NOTE(Parameter_VelNoisePitch),
    PER_NOTE(Parameter_VelToneOver),

    #undef PER_NOTE

    Parameter_Hat1Length,
    Parameter_Hat2Length,
    Parameter_Hat3Length,
    Parameter_HatPanWidth,

    Parameter_Tom1Pitch,
    Parameter_Tom2Pitch,
    Parameter_Tom3Pitch,
    Parameter_TomPanWidth,

    Parameter_OutputGain,

    Parameter_Count,

    Parameter_NoteLevelFirstGroup = Parameter_ToneLevel1,
    Parameter_NoteLevelLastGroup = Parameter_VelToneOver1,
};

enum {
    /* state IDs */
    State_ProgramName,

    State_Count,
};

struct ParameterName {
    std::string symbol;
    std::string name;
};

ParameterName GetParameterName(uint32_t index);
