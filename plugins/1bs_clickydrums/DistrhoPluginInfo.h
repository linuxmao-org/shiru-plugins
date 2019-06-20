#pragma once

#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "1bitstudio - clickydrums"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/1bs_clickydrums"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/1bs_clickydrums"
#define DISTRHO_PLUGIN_UNIQUE_ID       '1','c','l','d'
#define DISTRHO_PLUGIN_VERSION         0,13,0
#define DISTRHO_PLUGIN_LABEL           "1bitstudio - clickydrums"
#define DISTRHO_PLUGIN_LICENSE         "http://spdx.org/licenses/WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v0.13 03.06.19 [LinuxMAO]"
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
    pIdDuration##x, \
    pIdMin##x, \
    pIdMax##x, \
    pIdSeed##x, \
    pIdType##x, \
    pIdDrumVolume##x \

    PER_NOTE(1), PER_NOTE(2), PER_NOTE(3), PER_NOTE(4), PER_NOTE(5), PER_NOTE(6), \
    PER_NOTE(7), PER_NOTE(8), PER_NOTE(9), PER_NOTE(10), PER_NOTE(11), PER_NOTE(12),

    #undef PER_NOTE

    pIdPolyphony,
    pIdOutputGain,

    Parameter_Count,

    pIdNoteLevelFirstGroup = pIdDuration1
};

enum {
    /* state IDs */
    State_ProgramName,

    State_Count
};
