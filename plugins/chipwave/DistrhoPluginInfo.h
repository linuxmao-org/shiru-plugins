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
    pIdOscAWave,
    pIdOscADuty,
    pIdOscAOver,
    pIdOscACut,
    pIdOscAMultiple,
    pIdOscASeed,

    pIdOscBWave,
    pIdOscBDuty,
    pIdOscBOver,
    pIdOscBCut,
    pIdOscBDetune,
    pIdOscBMultiple,
    pIdOscBSeed,

    pIdOscBalance,
    pIdOscMixMode,

    pIdFltCutoff,
    pIdFltReso,

    pIdSlideDelay,
    pIdSlideSpeed,
    pIdSlideRoute,

    pIdEnvAttack,
    pIdEnvDecay,
    pIdEnvSustain,
    pIdEnvRelease,
    pIdEnvOscADepth,
    pIdEnvOscBDepth,
    pIdEnvOscBDetuneDepth,
    pIdEnvOscMixDepth,
    pIdEnvFltDepth,
    pIdEnvLfoDepth,

    pIdLfoSpeed,
    pIdLfoPitchDepth,
    pIdLfoOscADepth,
    pIdLfoOscBDepth,
    pIdLfoOscMixDepth,
    pIdLfoFltDepth,

    pIdAmpAttack,
    pIdAmpDecay,
    pIdAmpSustain,
    pIdAmpRelease,

    pIdVelAmp,
    pIdVelOscADepth,
    pIdVelOscBDepth,
    pIdVelOscMixDepth,
    pIdVelFltCutoff,
    pIdVelFltReso,

    pIdPolyphony,
    pIdPortaSpeed,
    pIdOutputGain,

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
