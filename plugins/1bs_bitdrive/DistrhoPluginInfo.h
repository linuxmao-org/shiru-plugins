#pragma once

#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "1bitstudio - bitdrive"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/1bs_bitdrive"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/1bs_bitdrive"
#define DISTRHO_PLUGIN_UNIQUE_ID       '1','b','b','d'
#define DISTRHO_PLUGIN_VERSION         0,12,0
#define DISTRHO_PLUGIN_LABEL           "1bitstudio - bitdrive"
#define DISTRHO_PLUGIN_LICENSE         "WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v0.12 02.06.19 [LinuxMAO]"
#define DISTRHO_PLUGIN_NUM_INPUTS      2
#define DISTRHO_PLUGIN_NUM_OUTPUTS     2
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
    pIdBypass,
    pIdInputGain,
    pIdThreshold,
    pIdOutputGain,

    Parameter_Count
};

enum {
    /* state IDs */
    State_Count
};
