#pragma once
#include "DistrhoPluginInfo.h"

__attribute__((unused)) static constexpr double sSampleRate[] = {
    4177.40,    //PAL 0..15
    4696.63,
    5261.41,
    5579.22,
    6023.94,
    7044.94,
    7917.18,
    8397.01,
    9446.63,
    11233.8,
    12595.5,
    14089.9,
    16965.4,
    21315.5,
    25191.0,
    33252.1,

    4181.71,    //NTSC 0..15
    4709.93,
    5264.04,
    5593.04,
    6257.95,
    7046.35,
    7919.35,
    8363.42,
    9419.86,
    11186.1,
    12604.0,
    13982.6,
    16884.6,
    21306.8,
    24858.0,
    33143.9
};

inline float hermite4(float frac_pos, float xm1, float x0, float x1, float x2)
{
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float b_neg = w + a;

    return ((((a * frac_pos) - b_neg) * frac_pos + c) * frac_pos + x0);
}
