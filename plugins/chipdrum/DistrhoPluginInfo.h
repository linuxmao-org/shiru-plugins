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

    PER_NOTE(pIdToneLevel),
    PER_NOTE(pIdToneDecay),
    PER_NOTE(pIdToneSustain),
    PER_NOTE(pIdToneRelease),
    PER_NOTE(pIdTonePitch),
    PER_NOTE(pIdToneSlide),
    PER_NOTE(pIdToneWave),
    PER_NOTE(pIdToneOver),

    PER_NOTE(pIdNoiseLevel),
    PER_NOTE(pIdNoiseDecay),
    PER_NOTE(pIdNoiseSustain),
    PER_NOTE(pIdNoiseRelease),
    PER_NOTE(pIdNoisePitch1),
    PER_NOTE(pIdNoisePitch2),
    PER_NOTE(pIdNoisePitch2Off),
    PER_NOTE(pIdNoisePitch2Len),
    PER_NOTE(pIdNoisePeriod),
    PER_NOTE(pIdNoiseSeed),
    PER_NOTE(pIdNoiseType),

    PER_NOTE(pIdRetrigTime),
    PER_NOTE(pIdRetrigCount),
    PER_NOTE(pIdRetrigRoute),

    PER_NOTE(pIdFilterLP),
    PER_NOTE(pIdFilterHP),
    PER_NOTE(pIdFilterRoute),

    PER_NOTE(pIdDrumGroup),
    PER_NOTE(pIdDrumBitDepth),
    PER_NOTE(pIdDrumUpdateRate),
    PER_NOTE(pIdDrumVolume),
    PER_NOTE(pIdDrumPan),

    PER_NOTE(pIdVelDrumVolume),
    PER_NOTE(pIdVelTonePitch),
    PER_NOTE(pIdVelNoisePitch),
    PER_NOTE(pIdVelToneOver),

    #undef PER_NOTE

    pIdHat1Length,
    pIdHat2Length,
    pIdHat3Length,
    pIdHatPanWidth,

    pIdTom1Pitch,
    pIdTom2Pitch,
    pIdTom3Pitch,
    pIdTomPanWidth,

    pIdOutputGain,

    Parameter_Count,

    pIdNoteLevelFirstGroup = pIdToneLevel1,
    pIdNoteLevelLastGroup = pIdVelToneOver1,
};

enum {
    /* state IDs */
    State_ProgramName,

    State_Count,
};
