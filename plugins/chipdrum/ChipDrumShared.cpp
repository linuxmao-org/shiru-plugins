#include "ChipDrumShared.hpp"
#include <random>
#include <cmath>

void InitNoise(uint8_t noise[65536])
{
    std::minstd_rand prng;
    prng.seed(1);
    for (unsigned i = 0; i < 65536; ++i) noise[i] = std::uniform_int_distribution<>{0x00, 0xff}(prng);
}

float FloatToHz(float value, float range)
{
    float hz;

    if(value<0.0f) value=0.0f;
    if(value>1.0f) value=1.0f;

    hz=-range*log10f(1.0f-value)/3.0f;

    if(hz<0) hz=0;
    if(hz>range) hz=range;

    return hz;
}

int FloatToNoisePeriod(float value)
{
    const int period[]={7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};

    return period[(int)(value*13.99f)];
}

float OverdriveValue(float value)
{
    return 1.0f+value*(OVERDRIVE_MAX-1.0f);
}

float SynthGetSample(int32_t wave, float acc, float over)
{
    float sample;

    float pi = M_PI;

    switch(wave)
    {
    case 0:    //square
        sample=acc<.5f?1.0f:0;
        break;

    case 1:    //saw
        sample=acc;
        break;

    case 2:    //tri
        sample=(((acc<.5f)?(acc/.5f):((1.0f-acc)/.5f))-.5f)*2.0f;
        break;

    case 3:    //sine
        sample=sinf(pi*2.0f*acc);
        break;

    default:
        sample=0;
    }

    sample*=over;

    if(sample<-1.0f) sample=-1.0f;
    if(sample> 1.0f) sample= 1.0f;

    return sample;
}

std::string GetParameterDisplay(uint32_t index, double value)
{
    std::string text;

    switch (ParameterFirstOfGroup(index))
    {
    case pIdToneLevel1:
        text = std::to_string(value); break;
    case pIdToneDecay1:
        text = std::to_string(DECAY_TIME_MAX_MS*value); break;
    case pIdToneSustain1:
        text = std::to_string(value); break;
    case pIdToneRelease1:
        text = std::to_string(RELEASE_TIME_MAX_MS*value); break;
    case pIdTonePitch1:
        text = std::to_string(FloatToHz(value,TONE_PITCH_MAX_HZ)); break;
    case pIdToneSlide1:
        text = std::to_string(-1.0f+value*2.0f); break;
    case pIdToneWave1:
        text=pWaveformNames[(int32_t)(value*3.99f)]; break;
    case pIdToneOver1:
        text = std::to_string(OverdriveValue(value)); break;

    case pIdNoiseLevel1:
        text = std::to_string(value); break;
    case pIdNoiseDecay1:
        text = std::to_string(DECAY_TIME_MAX_MS*value); break;
    case pIdNoiseSustain1:
        text = std::to_string(value); break;
    case pIdNoiseRelease1:
        text = std::to_string(RELEASE_TIME_MAX_MS*value); break;
    case pIdNoisePitch11:
        text = std::to_string((value*NOISE_PITCH_MAX_HZ)); break;
    case pIdNoisePitch21:
        text = std::to_string((value*NOISE_PITCH_MAX_HZ)); break;
    case pIdNoisePitch2Off1:
        text = std::to_string((value*DECAY_TIME_MAX_MS)); break;
    case pIdNoisePitch2Len1:
        text = std::to_string((value*NOISE_BURST_MAX_MS)); break;
    case pIdNoisePeriod1:
        text = std::to_string((FloatToNoisePeriod(value)+1)); break;
    case pIdNoiseSeed1:
        if(value>0) text = std::to_string(value); else text="Random"; break;
    case pIdNoiseType1:
        text = value<.5f?"White":"Digital"; break;

    case pIdRetrigTime1:
        text = std::to_string(RETRIGGER_MAX_MS*value); break;
    case pIdRetrigCount1:
        text = std::to_string((int32_t)(1+RETRIGGER_MAX_COUNT*value)); break;
    case pIdRetrigRoute1:
        text = pRetrigRouteNames[(int32_t)(value*2.99f)]; break;

    case pIdFilterLP1:
        if(value<1.0f) text = std::to_string(FILTER_CUTOFF_MAX_HZ*value); else text="Disabled"; break;
    case pIdFilterHP1:
        if(value>0.0f) text = std::to_string(FILTER_CUTOFF_MAX_HZ*value); else text="Disabled"; break;
    case pIdFilterRoute1:
        text = pFilterRouteNames[(int32_t)(value*3.99f)]; break;

    case pIdDrumGroup1:
        text = std::to_string((int32_t)(value*((DISTRHO_PLUGIN_NUM_OUTPUTS/2)-0.1f))); break;
    case pIdDrumBitDepth1:
        text = pBitDepthNames[(int32_t)(value*7.99f)]; break;
    case pIdDrumUpdateRate1:
        if(value<1.0f) text = std::to_string(MIN_UPDATE_RATE+value*MAX_UPDATE_RATE); else text="Analog"; break;
    case pIdDrumVolume1:
        text = std::to_string(20 * log10(value)) + " dB"; break;
    case pIdDrumPan1:
        text = std::to_string(value-.5f); break;

    case pIdVelDrumVolume1:
        text = std::to_string(value); break;
    case pIdVelTonePitch1:
        text = std::to_string(value*2.0f-1.0f); break;
    case pIdVelNoisePitch1:
        text = std::to_string(value*2.0f-1.0f); break;
    case pIdVelToneOver1:
        text = std::to_string(value*2.0f-1.0f); break;

    case pIdHat1Length:
        text = std::to_string(value*2.0f-1.0f); break;
    case pIdHat2Length:
        text = std::to_string(value*2.0f-1.0f); break;
    case pIdHat3Length:
        text = std::to_string(value*2.0f-1.0f); break;
    case pIdHatPanWidth:
        text = std::to_string(value*2.0f-1.0f); break;

    case pIdTom1Pitch:
        text = std::to_string(value*2.0f-1.0f);break;
    case pIdTom2Pitch:
        text = std::to_string(value*2.0f-1.0f); break;
    case pIdTom3Pitch:
        text = std::to_string(value*2.0f-1.0f); break;
    case pIdTomPanWidth:
        text = std::to_string(value*2.0f-1.0f); break;

    case pIdOutputGain:
        text = std::to_string(20 * log10(value)) + " dB"; break;
    }

    return text;
}
