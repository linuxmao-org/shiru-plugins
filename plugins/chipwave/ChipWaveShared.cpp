#include "ChipWaveShared.hpp"
#include "DistrhoPluginInfo.h"
#include <random>
#include <cmath>

void InitNoise(int32_t noise[65536])
{
    std::minstd_rand prng;
    prng.seed(1);
    for (unsigned i = 0; i < 65536; ++i) noise[i] = prng() & 1;
}

float OverdriveValue(float value)
{
    if(value<.5f) return value*2.0f; else return 1.0f+(value-.5f)*2.0f*(OVERDRIVE_MAX-1.0f);
}

float SynthGetSample(SynthOscObject *osc, const int32_t *noise, float over, float duty, int wave)
{
    const int32_t noise_mask[]={7,15,31,63,127,255,511,1023,2047,4095,8191,16383,32767,65535};
    int32_t mask;
    float out;

    const float pi = M_PI;

    switch(wave)
    {
    case 0://pulse
        {
            out=(osc->acc<duty)?0.0f:.5f;

            break;
        }

    case 1://tri/saw
        {
            out=((osc->acc<duty)?(osc->acc/duty):((1.0f-osc->acc)/(1.0f-duty)))-.5f;

            break;
        }

    case 2://sine
        {
            out=(osc->acc<duty)?(.5f*sinf(pi*(osc->acc/duty))):(-.5f*sinf(pi*((1.0f-osc->acc)/(1.0f-duty))));

            break;
        }

    case 3://noise
        {
            mask=(int)(duty*13.99f);

            out=noise[((osc->noise&noise_mask[mask])+osc->noise_seed)&65535]?.5f:0;

            break;
        }

    default:
        out=0;
    }

    out*=over;

    if(out> .5f) out= .5f;
    if(out<-.5f) out=-.5f;

    return out;
}

float SynthEnvelopeTimeToDelta(float value, float max_ms)
{
    if (value<.00005f) value=.00005f;

    return (1.0f/ENVELOPE_UPDATE_RATE_HZ)/(value*max_ms/1000.0f);
}

float FloatToMultiple(float value)
{
    const float mul[]={1.0f/128,1.0f/64,1.0f/32,1.0f/16,1.0f/8,1.0f/4,1.0f/2,1,2,4,8,16,32,64,128};
    return mul[(int)(value*14.99f)];
}

std::string GetParameterDisplay(uint32_t index, double value)
{
    std::string text;

    switch(index)
    {
    case Parameter_OscAWave: text = OscWaveformNames[(int)(value*3.99f)]; break;
    case Parameter_OscADuty: text = std::to_string(value*100.0f); break;
    case Parameter_OscAOver: text = std::to_string(OverdriveValue(value)); break;
    case Parameter_OscACut: if(value>0) text = std::to_string(OSC_CUT_MAX_MS*value); else text = "Inf"; break;
    case Parameter_OscAMultiple: text = std::to_string(FloatToMultiple(value)); break;
    case Parameter_OscASeed: if(value==0) text = "Random"; else text = std::to_string(value); break;

    case Parameter_OscBWave: text = OscWaveformNames[(int)(value*3.99f)]; break;
    case Parameter_OscBDuty: text = std::to_string(value*100.0f); break;
    case Parameter_OscBOver: text = std::to_string(OverdriveValue(value)); break;
    case Parameter_OscBCut: if(value>0) text = std::to_string(OSC_CUT_MAX_MS*value); else text = "Inf"; break;
    case Parameter_OscBDetune: text = std::to_string(value*DETUNE_SEMITONES); break;
    case Parameter_OscBMultiple: text = std::to_string(FloatToMultiple(value)); break;
    case Parameter_OscBSeed: if(value==0) text = "Random"; else text = std::to_string(value); break;

    case Parameter_OscBalance: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_OscMixMode: text = value<.5f?"Add":"Mul"; break;

    case Parameter_FltCutoff: if(value<1.0f) text = std::to_string(value*FILTER_CUTOFF_MAX_HZ); else text = "Disabled";; break;
    case Parameter_FltReso: text = std::to_string(value); break;

    case Parameter_SlideDelay: if(value<1.0f) text = std::to_string(MOD_DELAY_MAX_MS*value); else text = "Key Off"; break;
    case Parameter_SlideSpeed: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_SlideRoute: text = SlideRouteNames[(int)(value*2.99f)]; break;

    case Parameter_EnvAttack: text = std::to_string(ENVELOPE_ATTACK_MAX_MS*value); break;
    case Parameter_EnvDecay: text = std::to_string(ENVELOPE_DECAY_MAX_MS*value); break;
    case Parameter_EnvSustain: text = std::to_string(value); break;
    case Parameter_EnvRelease: text = std::to_string(ENVELOPE_RELEASE_MAX_MS*value); break;

    case Parameter_EnvOscADepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_EnvOscBDepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_EnvOscBDetuneDepth: text = std::to_string(value); break;
    case Parameter_EnvOscMixDepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_EnvFltDepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_EnvLfoDepth: text = std::to_string(value); break;

    case Parameter_AmpAttack: text = std::to_string(ENVELOPE_ATTACK_MAX_MS*value); break;
    case Parameter_AmpDecay: text = std::to_string(ENVELOPE_DECAY_MAX_MS*value); break;
    case Parameter_AmpSustain: text = std::to_string(value); break;
    case Parameter_AmpRelease: text = std::to_string(ENVELOPE_RELEASE_MAX_MS*value); break;

    case Parameter_LfoSpeed: text = std::to_string(value*LFO_MAX_HZ); break;
    case Parameter_LfoPitchDepth: text = std::to_string(value); break;
    case Parameter_LfoOscADepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_LfoOscBDepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_LfoOscMixDepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_LfoFltDepth: text = std::to_string(value*2.0f-1.0f); break;

    case Parameter_VelAmp: text = std::to_string(value); break;
    case Parameter_VelOscADepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_VelOscBDepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_VelOscMixDepth: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_VelFltCutoff: text = std::to_string(value*2.0f-1.0f); break;
    case Parameter_VelFltReso: text = std::to_string(value*2.0f-1.0f); break;


    case Parameter_Polyphony: text = value<.5f?"Mono":"Poly"; break;
    case Parameter_PortaSpeed: text = std::to_string(value); break;
    case Parameter_OutputGain: text = std::to_string(20 * log10(value)) + " dB"; break;
    }

    return text;
}
