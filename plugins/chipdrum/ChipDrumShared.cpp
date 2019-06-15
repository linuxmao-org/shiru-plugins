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
    case Parameter_ToneLevel1:
        text = std::to_string(value); break;
    case Parameter_ToneDecay1:
        text = std::to_string(DECAY_TIME_MAX_MS*value); break;
    case Parameter_ToneSustain1:
        text = std::to_string(value); break;
    case Parameter_ToneRelease1:
        text = std::to_string(RELEASE_TIME_MAX_MS*value); break;
    case Parameter_TonePitch1:
        text = std::to_string(FloatToHz(value,TONE_PITCH_MAX_HZ)); break;
    case Parameter_ToneSlide1:
        text = std::to_string(-1.0f+value*2.0f); break;
    case Parameter_ToneWave1:
        text=pWaveformNames[(int32_t)(value*3.99f)]; break;
    case Parameter_ToneOver1:
        text = std::to_string(OverdriveValue(value)); break;

    case Parameter_NoiseLevel1:
        text = std::to_string(value); break;
    case Parameter_NoiseDecay1:
        text = std::to_string(DECAY_TIME_MAX_MS*value); break;
    case Parameter_NoiseSustain1:
        text = std::to_string(value); break;
    case Parameter_NoiseRelease1:
        text = std::to_string(RELEASE_TIME_MAX_MS*value); break;
    case Parameter_NoisePitch11:
        text = std::to_string((value*NOISE_PITCH_MAX_HZ)); break;
    case Parameter_NoisePitch21:
        text = std::to_string((value*NOISE_PITCH_MAX_HZ)); break;
    case Parameter_NoisePitch2Off1:
        text = std::to_string((value*DECAY_TIME_MAX_MS)); break;
    case Parameter_NoisePitch2Len1:
        text = std::to_string((value*NOISE_BURST_MAX_MS)); break;
    case Parameter_NoisePeriod1:
        text = std::to_string((FloatToNoisePeriod(value)+1)); break;
    case Parameter_NoiseSeed1:
        if(value>0) text = std::to_string(value); else text="Random"; break;
    case Parameter_NoiseType1:
        text = value<.5f?"White":"Digital"; break;

    case Parameter_RetrigTime1:
        text = std::to_string(RETRIGGER_MAX_MS*value); break;
    case Parameter_RetrigCount1:
        text = std::to_string((int32_t)(1+RETRIGGER_MAX_COUNT*value)); break;
    case Parameter_RetrigRoute1:
        text = pRetrigRouteNames[(int32_t)(value*2.99f)]; break;

    case Parameter_FilterLP1:
        if(value<1.0f) text = std::to_string(FILTER_CUTOFF_MAX_HZ*value); else text="Disabled"; break;
    case Parameter_FilterHP1:
        if(value>0.0f) text = std::to_string(FILTER_CUTOFF_MAX_HZ*value); else text="Disabled"; break;
    case Parameter_FilterRoute1:
        text = pFilterRouteNames[(int32_t)(value*3.99f)]; break;

    case Parameter_DrumGroup1:
        text = std::to_string((int32_t)(value*((DISTRHO_PLUGIN_NUM_OUTPUTS/2)-0.1f))); break;
    case Parameter_DrumBitDepth1:
        text = pBitDepthNames[(int32_t)(value*7.99f)]; break;
    case Parameter_DrumUpdateRate1:
        if(value<1.0f) text = std::to_string(MIN_UPDATE_RATE+value*MAX_UPDATE_RATE); else text="Analog"; break;
    case Parameter_DrumVolume1:
        text = std::to_string(20 * log10(value)) + " dB"; break;
    case Parameter_DrumPan1:
        text = std::to_string(value-.5f); break;

    case Parameter_VelDrumVolume1:
        text = std::to_string(value); break;
    case Parameter_VelTonePitch1:
        text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_VelNoisePitch1:
        text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_VelToneOver1:
        text = std::to_string(value*2.0f-1.0f); break;

    case Parameter_Hat1Length:
        text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_Hat2Length:
        text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_Hat3Length:
        text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_HatPanWidth:
        text = std::to_string(value*2.0f-1.0f); break;

    case Parameter_Tom1Pitch:
        text = std::to_string(value*2.0f-1.0f);break;
    case Parameter_Tom2Pitch:
        text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_Tom3Pitch:
        text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_TomPanWidth:
        text = std::to_string(value*2.0f-1.0f); break;

    case Parameter_OutputGain:
        text = std::to_string(20 * log10(value)) + " dB"; break;
    }

    return text;
}
