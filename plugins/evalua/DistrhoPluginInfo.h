#pragma once
#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "EVALUA"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/evalua"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/evalua"
#define DISTRHO_PLUGIN_UNIQUE_ID       'e','v','a','u'
#define DISTRHO_PLUGIN_VERSION         1,1,0
#define DISTRHO_PLUGIN_LABEL           "EVALUA"
#define DISTRHO_PLUGIN_LICENSE         "http://spdx.org/licenses/WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v1.01 30.05.19 Shiru [LinuxMAO]"
#define DISTRHO_PLUGIN_NUM_INPUTS      0
#define DISTRHO_PLUGIN_NUM_OUTPUTS     2
#define DISTRHO_PLUGIN_IS_SYNTH        1
#define DISTRHO_PLUGIN_HAS_UI          1
#define DISTRHO_PLUGIN_HAS_EMBED_UI    1
#define DISTRHO_PLUGIN_HAS_EXTERNAL_UI 0
#define DISTRHO_PLUGIN_IS_RT_SAFE      1
#define DISTRHO_PLUGIN_WANT_PROGRAMS   1
#define DISTRHO_PLUGIN_WANT_STATE      1
#define DISTRHO_PLUGIN_WANT_FULL_STATE 1
#define DISTRHO_PLUGIN_NUM_PROGRAMS    PresetDataLength

enum {
    /* parameter IDs */
    Parameter_Count
};

enum {
    /* state IDs */
    State_ProgramName,
    State_ProgramData,
    State_Polyphony,
    State_PortaSpeed,
    State_OutputGain,
    State_Count,
};
