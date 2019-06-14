#pragma once
#include <cstdint>

#define DISTRHO_PLUGIN_BRAND           "Shiru"
#define DISTRHO_PLUGIN_NAME            "ChipWave"
#define DISTRHO_PLUGIN_URI             "https://github.com/linuxmao-org/shiru-plugins/chipwave"
#define DISTRHO_PLUGIN_HOMEPAGE        "https://github.com/linuxmao-org/shiru-plugins/chipwave"
#define DISTRHO_PLUGIN_UNIQUE_ID       'c','h','w','a'
#define DISTRHO_PLUGIN_VERSION         0,72,0
#define DISTRHO_PLUGIN_LABEL           "ChipWave"
#define DISTRHO_PLUGIN_LICENSE         "WTFPL"
#define DISTRHO_PLUGIN_MAKER           "Shiru"
#define DISTRHO_PLUGIN_DESCRIPTION     "v0.72 03.06.19 [LinuxMAO]"
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
#define DISTRHO_PLUGIN_NUM_PROGRAMS    PresetData.size()

enum {
    Parameter_OscAWave,
    Parameter_OscADuty,
    Parameter_OscAOver,
    Parameter_OscACut,
    Parameter_OscAMultiple,
    Parameter_OscASeed,

    Parameter_OscBWave,
    Parameter_OscBDuty,
    Parameter_OscBOver,
    Parameter_OscBCut,
    Parameter_OscBDetune,
    Parameter_OscBMultiple,
    Parameter_OscBSeed,

    Parameter_OscBalance,
    Parameter_OscMixMode,

    Parameter_FltCutoff,
    Parameter_FltReso,

    Parameter_SlideDelay,
    Parameter_SlideSpeed,
    Parameter_SlideRoute,

    Parameter_EnvAttack,
    Parameter_EnvDecay,
    Parameter_EnvSustain,
    Parameter_EnvRelease,
    Parameter_EnvOscADepth,
    Parameter_EnvOscBDepth,
    Parameter_EnvOscBDetuneDepth,
    Parameter_EnvOscMixDepth,
    Parameter_EnvFltDepth,
    Parameter_EnvLfoDepth,

    Parameter_LfoSpeed,
    Parameter_LfoPitchDepth,
    Parameter_LfoOscADepth,
    Parameter_LfoOscBDepth,
    Parameter_LfoOscMixDepth,
    Parameter_LfoFltDepth,

    Parameter_AmpAttack,
    Parameter_AmpDecay,
    Parameter_AmpSustain,
    Parameter_AmpRelease,

    Parameter_VelAmp,
    Parameter_VelOscADepth,
    Parameter_VelOscBDepth,
    Parameter_VelOscMixDepth,
    Parameter_VelFltCutoff,
    Parameter_VelFltReso,

    Parameter_Polyphony,
    Parameter_PortaSpeed,
    Parameter_OutputGain,

    Parameter_Count
};

enum {
    /* state IDs */
    State_ProgramName,

    State_Count,
};

struct ParameterName {
    const char *symbol;
    const char *name;
};

ParameterName GetParameterName(uint32_t index);
