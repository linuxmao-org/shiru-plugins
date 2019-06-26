#pragma once

#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "1bitstudio - noisedrums"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/1bs_noisedrums"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/1bs_noisedrums"
#define DISTRHO_PLUGIN_UNIQUE_ID       '1','n','s','d'
#define DISTRHO_PLUGIN_VERSION         0,14,0
#define DISTRHO_PLUGIN_LABEL           "1bitstudio - noisedrums"
#define DISTRHO_PLUGIN_LICENSE         "http://spdx.org/licenses/WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v0.14 03.06.19 [LinuxMAO]"
#define DISTRHO_PLUGIN_NUM_INPUTS      0
#define DISTRHO_PLUGIN_NUM_OUTPUTS     2
#define DISTRHO_PLUGIN_IS_SYNTH        1
#define DISTRHO_PLUGIN_HAS_UI          0
#define DISTRHO_PLUGIN_HAS_EMBED_UI    0
#define DISTRHO_PLUGIN_HAS_EXTERNAL_UI 0
#define DISTRHO_PLUGIN_IS_RT_SAFE      1
#define DISTRHO_PLUGIN_WANT_PROGRAMS   1
#define DISTRHO_PLUGIN_WANT_STATE      1
#define DISTRHO_PLUGIN_WANT_FULL_STATE 1
#define DISTRHO_PLUGIN_NUM_PROGRAMS    PresetData.size()

enum {
    /* parameter IDs */

    #define PER_NOTE(x) \
    pIdDrumVolume##x, \
    PER_BURST(x, 1), PER_BURST(x, 2), PER_BURST(x, 3), PER_BURST(x, 4), \
    PER_BURST(x, 5), PER_BURST(x, 6), PER_BURST(x, 7), PER_BURST(x, 8)

    #define PER_BURST(x, y) \
    pIdBurst##y##Duration##x, \
    pIdBurst##y##Pitch##x, \
    pIdBurst##y##Period##x, \
    pIdBurst##y##PulseWidth##x

    PER_NOTE(1), PER_NOTE(2), PER_NOTE(3), PER_NOTE(4), PER_NOTE(5), PER_NOTE(6), \
    PER_NOTE(7), PER_NOTE(8), PER_NOTE(9), PER_NOTE(10), PER_NOTE(11), PER_NOTE(12),

    #undef PER_NOTE
    #undef PER_BURST

    pIdPolyphony,
    pIdOutputGain,

    Parameter_Count,

    pIdNoteLevelFirstGroup = pIdDrumVolume1,
    pIdBurstLevelFirstGroup = pIdBurst1Duration1
};

enum {
    /* state IDs */
    State_ProgramName,

    State_Count
};
