#pragma once
#include "DistrhoPluginInfo.h"
#include <array>

enum {
    MaxNameLen = 32
};

struct Preset {
    char name[MaxNameLen + 1];
    float values[Parameter_Count];
};

__attribute__((unused))
static const std::array<Preset, 52> PresetData = {
    /*   0 */ Preset{"Chip square 50%",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   1 */ Preset{"Chip square 25%",
     {0.000000, 0.250000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   2 */ Preset{"Chip square 12.5%",
     {0.000000, 0.125000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   3 */ Preset{"Chip triangle",
     {0.500000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   4 */ Preset{"Chip saw",
     {0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   5 */ Preset{"AY envelope E 1:1",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.500000, 0.500000, 0.000000, 0.006000, 0.500000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   6 */ Preset{"AY envelope E 1:2",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.500000, 0.500000, 0.000000, 0.003000, 0.449000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   7 */ Preset{"AY envelope C 1:1",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 1.000000, 0.500000, 0.000000, 0.003000, 0.500000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   8 */ Preset{"AY envelope C 1:2",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 1.000000, 0.500000, 0.000000, 0.006000, 0.454000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*   9 */ Preset{"Square kick",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 1.000000, 0.000000, 0.000000, 0.148000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.009000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  10 */ Preset{"Square/noise snare",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 1.000000, 0.164000, 0.500000, 0.000000, 0.000000, 0.765000, 0.000000, 0.410714, 0.000000, 1.000000, 0.000000, 0.000000, 0.266000, 0.483000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.008000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  11 */ Preset{"Square/noise snare 2",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 1.000000, 1.000000, 0.500000, 0.053571, 0.000000, 0.737000, 0.000000, 0.358000, 0.000000, 1.000000, 0.000000, 0.000000, 0.432000, 0.500000, 0.000000, 0.006000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.017000, 0.000000, 0.171000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  12 */ Preset{"Theremin like solo",
     {0.681000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.131000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 1.000000, 0.297000, 0.394000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.777000, 1.000000}},
    /*  13 */ Preset{"Square to sine",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.737000, 0.500000, 0.500000, 0.000000, 0.000000, 0.504000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.564000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 1.000000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.100000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  14 */ Preset{"Key 1",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.576000, 0.500000, 0.500000, 0.000000, 0.006000, 0.500000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.121000, 0.123000, 0.372000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  15 */ Preset{"Filtered saw bass",
     {0.441000, 1.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 0.011000, 0.079000, 0.000000, 0.500000, 0.000000, 0.000000, 0.019000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 1.000000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.077000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  16 */ Preset{"Weird pad",
     {0.000000, 0.935000, 0.500000, 0.000000, 0.500000, 0.000000, 0.515000, 0.090000, 0.500000, 0.000000, 0.003000, 0.551000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.233000, 0.230000, 0.214000, 0.178000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  17 */ Preset{"Filtered pad",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.244000, 0.500000, 0.000000, 0.009000, 0.500000, 0.000000, 0.500000, 1.000000, 0.027000, 0.139000, 0.000000, 0.500000, 0.000000, 0.168000, 0.360000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 1.000000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.399000, 0.246000, 0.146000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  18 */ Preset{"Synth solo",
     {0.201000, 0.275000, 0.500000, 0.000000, 0.500000, 0.000000, 0.717000, 0.710000, 0.500000, 0.000000, 0.027357, 0.500000, 0.000000, 0.500000, 1.000000, 0.336000, 0.115000, 1.000000, 0.393000, 0.000000, 0.308000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.493000, 1.000000, 0.189000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.846000, 1.000000}},
    /*  19 */ Preset{"Another pad",
     {0.500000, 0.002000, 0.500000, 0.000000, 0.500000, 0.000000, 0.449000, 0.000000, 0.500000, 0.000000, 0.012000, 0.500000, 0.000000, 0.500000, 1.000000, 0.000000, 0.019000, 0.000000, 0.500000, 0.000000, 0.147000, 0.169000, 0.211000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.762000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.172000, 0.509000, 0.000000, 0.297000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  20 */ Preset{"Key 2",
     {0.327000, 0.164000, 0.500000, 0.000000, 0.500000, 0.000000, 0.366000, 0.284000, 0.500000, 0.000000, 0.006000, 0.464000, 0.000000, 0.317000, 0.000000, 0.184000, 0.324000, 0.000000, 0.500000, 0.000000, 0.003000, 0.133000, 0.000000, 0.000000, 0.427000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.168000, 0.081000, 0.336000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  21 */ Preset{"Pad",
     {0.405000, 0.061000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.401000, 0.500000, 0.000000, 0.003000, 0.500000, 0.000000, 0.500000, 1.000000, 0.012000, 0.051000, 0.000000, 0.500000, 0.000000, 0.313000, 0.314000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.584000, 1.000000, 0.236000, 0.194000, 0.500000, 0.500000, 0.500000, 0.500000, 0.044000, 0.538000, 0.174000, 0.444000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  22 */ Preset{"Key slide down",
     {0.000000, 0.080000, 0.500000, 0.000000, 0.500000, 0.000000, 0.711000, 0.161000, 0.500000, 0.000000, 0.012000, 0.500000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 1.000000, 0.412000, 0.000000, 0.000000, 0.064000, 0.000000, 0.000000, 0.711000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.128000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  23 */ Preset{"Bass",
     {0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.017000, 0.500000, 0.000000, 0.000000, 0.545000, 0.000000, 0.267857, 1.000000, 0.018000, 0.034000, 0.000000, 0.500000, 0.000000, 0.000000, 0.012000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.601000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.112000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  24 */ Preset{"Glitch",
     {0.870000, 0.159000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.161000, 0.500000, 0.000000, 0.000000, 0.760000, 0.000000, 0.500000, 1.000000, 0.164000, 0.256000, 0.000000, 0.500000, 0.000000, 0.000000, 0.337000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.525000, 0.456000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.301000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  25 */ Preset{"Key",
     {0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.003000, 0.580000, 0.000000, 0.386000, 0.000000, 0.038000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.230000, 0.000000, 0.210000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  26 */ Preset{"Pad",
     {0.499000, 0.035000, 0.500000, 0.000000, 0.500000, 0.000000, 0.638000, 0.038000, 0.500000, 0.000000, 0.015000, 0.434000, 0.000000, 0.500000, 0.000000, 0.113000, 0.000000, 0.000000, 0.500000, 0.000000, 0.285000, 0.000000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.753000, 0.707000, 1.000000, 0.306000, 0.148000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  27 */ Preset{"Some sound",
     {0.000000, 0.090000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.500000, 0.500000, 0.000000, 0.012000, 0.500000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.241000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 1.000000, 0.255000, 0.091000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  28 */ Preset{"Harsh",
     {0.000000, 0.285000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.009000, 0.387000, 0.000000, 0.500000, 1.000000, 0.022000, 0.048000, 0.000000, 0.500000, 0.000000, 0.025000, 0.006000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 1.000000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.092000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  29 */ Preset{"Nasal solo",
     {0.000000, 0.892000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.082000, 0.500000, 0.000000, 0.003000, 0.424000, 0.000000, 0.500000, 1.000000, 0.086000, 0.457000, 0.000000, 0.500000, 0.000000, 0.262000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 1.000000, 0.321000, 0.380000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.742000, 1.000000}},
    /*  30 */ Preset{"Dubstep bass",
     {0.321000, 0.032000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.146000, 0.500000, 0.000000, 0.004000, 0.439000, 0.000000, 0.500000, 1.000000, 0.017000, 0.036000, 0.000000, 0.500000, 0.000000, 0.000000, 0.036000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 1.000000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  31 */ Preset{"Dynamic pad",
     {0.009000, 0.179000, 0.500000, 0.000000, 0.500000, 0.000000, 0.518000, 0.856000, 0.500000, 0.000000, 0.009000, 0.545000, 0.000000, 0.500000, 1.000000, 0.025000, 0.000000, 0.000000, 0.500000, 0.000000, 0.274000, 0.274000, 0.174000, 0.000000, 0.844000, 1.000000, 0.000000, 0.500000, 1.000000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.266000, 0.417000, 0.816000, 0.224000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  32 */ Preset{"Key",
     {0.140000, 0.278000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.729000, 0.500000, 0.000000, 0.007000, 0.554000, 0.000000, 0.499000, 1.000000, 0.141000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.077000, 0.000000, 0.000000, 0.500000, 0.595000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.061000, 0.055000, 0.195000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  33 */ Preset{"Wah bass",
     {0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.541000, 0.500000, 0.000000, 0.008000, 0.500000, 0.000000, 0.500000, 1.000000, 0.008000, 0.077000, 0.000000, 0.500000, 0.000000, 0.015000, 0.005000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.515000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  34 */ Preset{"Vanilla synth bass",
     {0.500000, 0.807000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.004000, 0.448000, 0.000000, 0.500000, 0.000000, 0.025000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.113000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.605000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  35 */ Preset{"Drone bass",
     {0.500000, 1.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.089000, 0.873000, 0.500000, 0.000000, 0.004000, 0.500000, 0.000000, 0.500000, 1.000000, 0.117000, 0.096000, 0.000000, 0.500000, 0.000000, 0.187000, 0.000000, 0.000000, 0.000000, 0.500000, 0.137000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  36 */ Preset{"Wide triangle",
     {0.500000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.500000, 0.500000, 0.500000, 0.000000, 0.022000, 0.500000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  37 */ Preset{"Vanilla bass 2",
     {0.000000, 0.548000, 0.500000, 0.000000, 0.500000, 0.000000, 0.477000, 0.000000, 0.500000, 0.000000, 0.016000, 0.500000, 0.000000, 0.500000, 1.000000, 0.006000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.035000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.541000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  38 */ Preset{"Sparkly piano",
     {0.545000, 0.464286, 0.500000, 0.000000, 0.500000, 0.000000, 0.476000, 0.500000, 0.500000, 0.000000, 0.012000, 0.928571, 0.000000, 0.419643, 0.000000, 0.111000, 0.000000, 0.000000, 0.500000, 0.000000, 0.077000, 0.077000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.091000, 0.100000, 0.237000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  39 */ Preset{"Pulsating drone",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.000000, 0.586000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.289000, 0.000000, 0.205000, 0.386000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  40 */ Preset{"Simmonds tom",
     {0.388000, 0.267000, 0.500000, 0.000000, 0.500000, 0.000000, 1.000000, 1.000000, 0.500000, 0.053571, 0.000000, 1.000000, 0.000000, 0.389000, 0.000000, 1.000000, 0.000000, 0.000000, 0.422000, 0.412000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.058000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  41 */ Preset{"Piano",
     {0.333333, 0.294643, 0.500000, 0.000000, 0.500000, 0.000000, 0.666667, 0.178571, 0.500000, 0.000000, 0.014000, 0.500000, 0.000000, 0.500000, 1.000000, 0.137000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.023000, 0.000000, 0.859000, 0.598000, 0.500000, 0.500000, 0.000000, 0.071429, 0.026786, 0.473214, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  42 */ Preset{"Synth",
     {0.462000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.112000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 1.000000, 0.174000, 0.241000, 0.578000, 0.500000, 0.500000, 0.500000, 0.000000, 0.142000, 0.051000, 0.193000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  43 */ Preset{"Modulated pad",
     {0.450000, 0.890000, 0.500000, 0.000000, 0.500000, 0.000000, 0.488000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.050000, 0.213000, 0.000000, 0.500000, 0.000000, 0.246000, 0.246000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.727000, 0.000000, 0.326000, 0.000000, 0.380000, 0.500000, 0.406000, 0.500000, 0.213000, 0.215000, 0.500000, 0.184000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  44 */ Preset{"Metallic hit",
     {0.000000, 0.500000, 0.500000, 0.000000, 0.748000, 0.000000, 1.000000, 1.000000, 0.500000, 0.044643, 0.000000, 0.776000, 0.000000, 0.500000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.009000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  45 */ Preset{"Modulated saw bass",
     {0.350000, 0.000000, 0.500000, 0.000000, 0.500000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.073000, 0.000000, 0.757000, 0.500000, 0.500000, 0.500000, 0.000000, 0.393000, 0.270000, 0.175000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  46 */ Preset{"Periodic glitch",
     {0.500000, 0.210000, 0.500000, 0.000000, 0.372000, 0.000000, 0.942000, 1.000000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 1.000000, 0.004000, 0.165000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.055000, 0.000000, 0.500000, 0.053000, 0.500000, 0.622000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  47 */ Preset{"Laser",
     {0.577000, 1.000000, 0.500000, 0.000000, 0.821000, 0.000000, 0.662000, 0.500000, 0.500000, 0.000000, 0.033000, 0.762000, 0.000000, 0.500000, 0.000000, 0.009000, 0.000000, 0.000000, 0.326000, 0.000000, 0.000000, 0.039000, 0.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 1.000000, 0.000000, 0.000000, 0.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.078000, 0.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  48 */ Preset{"AY env like",
     {0.666667, 0.857143, 0.500000, 0.000000, 0.500000, 0.000000, 0.333333, 0.000000, 0.500000, 0.000000, 0.017857, 0.500000, 0.000000, 0.857143, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  49 */ Preset{"Vibrating saw bass",
     {0.333333, 1.000000, 0.544643, 0.000000, 0.500000, 0.000000, 0.333333, 0.000000, 1.000000, 0.000000, 0.003500, 0.571429, 0.000000, 0.357143, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.294643, 0.125000, 0.955357, 0.580357, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  50 */ Preset{"Periodic noise tone",
     {1.000000, 0.000000, 0.500000, 0.000000, 0.571429, 0.285714, 1.000000, 0.000000, 0.500000, 0.000000, 0.017857, 0.571429, 0.294643, 0.500000, 1.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.000000, 0.125000, 0.500000, 0.500000, 0.500000, 0.500000, 0.000000, 0.000000, 1.000000, 0.000000, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
    /*  51 */ Preset{"Synth keyboard",
     {0.333333, 0.785714, 0.500000, 0.000000, 0.500000, 0.000000, 0.666667, 0.821429, 0.500000, 0.000000, 0.017857, 0.500000, 0.000000, 0.500000, 0.000000, 1.000000, 0.000000, 0.000000, 0.500000, 0.000000, 0.000000, 0.000000, 1.000000, 0.000000, 0.500000, 0.500000, 0.000000, 0.500000, 0.500000, 0.000000, 0.169643, 0.000000, 0.616071, 0.348214, 0.500000, 0.500000, 0.000000, 0.107143, 0.080357, 0.437500, 1.000000, 0.500000, 0.500000, 0.500000, 0.500000, 0.500000, 1.000000, 1.000000, 1.000000}},
};