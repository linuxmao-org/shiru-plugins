#pragma once
#include <cstdint>

#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "Flopster"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/flopster"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/flopster"
#define DISTRHO_PLUGIN_UNIQUE_ID       'f','s','t','r'
#define DISTRHO_PLUGIN_VERSION         1,1,0
#define DISTRHO_PLUGIN_LABEL           "Flopster"
#define DISTRHO_PLUGIN_LICENSE         "WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v1.01 03.06.19 [LinuxMAO]"
#define DISTRHO_PLUGIN_NUM_INPUTS      0
#define DISTRHO_PLUGIN_NUM_OUTPUTS     2
#define DISTRHO_PLUGIN_IS_SYNTH        1
#define DISTRHO_PLUGIN_HAS_UI          0
#define DISTRHO_PLUGIN_HAS_EMBED_UI    0
#define DISTRHO_PLUGIN_HAS_EXTERNAL_UI 0
#define DISTRHO_PLUGIN_IS_RT_SAFE      1
#define DISTRHO_PLUGIN_WANT_PROGRAMS   0
#define DISTRHO_PLUGIN_WANT_STATE      1
#define DISTRHO_PLUGIN_WANT_FULL_STATE 1
#define DISTRHO_PLUGIN_NUM_PROGRAMS    0

enum {
    /* parameter IDs */
    pIdHeadStepGain,
    pIdHeadSeekGain,
    pIdHeadBuzzGain,
    pIdSpindleGain,
    pIdNoisesGain,
    pIdOutputGain,

    Parameter_Count
};

enum {
    /* state IDs */
    State_Count,
};
