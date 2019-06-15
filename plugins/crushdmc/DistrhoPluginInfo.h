#pragma once

#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "CrushDMC"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/crushdmc"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/crushdmc"
#define DISTRHO_PLUGIN_UNIQUE_ID       'c','d','m','c'
#define DISTRHO_PLUGIN_VERSION         0,11,0
#define DISTRHO_PLUGIN_LABEL           "CrushDMC"
#define DISTRHO_PLUGIN_LICENSE         "WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v0.11 02.06.19 [LinuxMAO]"
#define DISTRHO_PLUGIN_NUM_INPUTS      1
#define DISTRHO_PLUGIN_NUM_OUTPUTS     1
#define DISTRHO_PLUGIN_IS_SYNTH        0
#define DISTRHO_PLUGIN_HAS_UI          0
#define DISTRHO_PLUGIN_HAS_EMBED_UI    0
#define DISTRHO_PLUGIN_HAS_EXTERNAL_UI 0
#define DISTRHO_PLUGIN_IS_RT_SAFE      1
#define DISTRHO_PLUGIN_WANT_PROGRAMS   0
#define DISTRHO_PLUGIN_WANT_STATE      0
#define DISTRHO_PLUGIN_WANT_FULL_STATE 0
#define DISTRHO_PLUGIN_NUM_PROGRAMS    0

enum {
    /* parameter IDs */
    Parameter_Bypass,
    Parameter_InputGain,
    Parameter_SampleRate,
    Parameter_OutputGain,

    Parameter_Count
};

enum {
    /* state IDs */
    State_Count
};
