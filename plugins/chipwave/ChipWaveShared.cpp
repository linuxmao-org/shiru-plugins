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
    case pIdOscAWave: text = OscWaveformNames[(int)(value*3.99f)]; break;
    case pIdOscADuty: text = std::to_string(value*100.0f); break;
    case pIdOscAOver: text = std::to_string(OverdriveValue(value)); break;
    case pIdOscACut: if(value>0) text = std::to_string(OSC_CUT_MAX_MS*value); else text = "Inf"; break;
    case pIdOscAMultiple: text = std::to_string(FloatToMultiple(value)); break;
    case pIdOscASeed: if(value==0) text = "Random"; else text = std::to_string(value); break;

    case pIdOscBWave: text = OscWaveformNames[(int)(value*3.99f)]; break;
    case pIdOscBDuty: text = std::to_string(value*100.0f); break;
    case pIdOscBOver: text = std::to_string(OverdriveValue(value)); break;
    case pIdOscBCut: if(value>0) text = std::to_string(OSC_CUT_MAX_MS*value); else text = "Inf"; break;
    case pIdOscBDetune: text = std::to_string(value*DETUNE_SEMITONES); break;
    case pIdOscBMultiple: text = std::to_string(FloatToMultiple(value)); break;
    case pIdOscBSeed: if(value==0) text = "Random"; else text = std::to_string(value); break;

    case pIdOscBalance: text = std::to_string(value*2.0f-1.0f); break;
    case pIdOscMixMode: text = value<.5f?"Add":"Mul"; break;

    case pIdFltCutoff: if(value<1.0f) text = std::to_string(value*FILTER_CUTOFF_MAX_HZ); else text = "Disabled"; break;
    case pIdFltReso: text = std::to_string(value); break;

    case pIdSlideDelay: if(value<1.0f) text = std::to_string(MOD_DELAY_MAX_MS*value); else text = "Key Off"; break;
    case pIdSlideSpeed: text = std::to_string(value*2.0f-1.0f); break;
    case pIdSlideRoute: text = SlideRouteNames[(int)(value*2.99f)]; break;

    case pIdEnvAttack: text = std::to_string(ENVELOPE_ATTACK_MAX_MS*value); break;
    case pIdEnvDecay: text = std::to_string(ENVELOPE_DECAY_MAX_MS*value); break;
    case pIdEnvSustain: text = std::to_string(value); break;
    case pIdEnvRelease: text = std::to_string(ENVELOPE_RELEASE_MAX_MS*value); break;

    case pIdEnvOscADepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdEnvOscBDepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdEnvOscBDetuneDepth: text = std::to_string(value); break;
    case pIdEnvOscMixDepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdEnvFltDepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdEnvLfoDepth: text = std::to_string(value); break;

    case pIdAmpAttack: text = std::to_string(ENVELOPE_ATTACK_MAX_MS*value); break;
    case pIdAmpDecay: text = std::to_string(ENVELOPE_DECAY_MAX_MS*value); break;
    case pIdAmpSustain: text = std::to_string(value); break;
    case pIdAmpRelease: text = std::to_string(ENVELOPE_RELEASE_MAX_MS*value); break;

    case pIdLfoSpeed: text = std::to_string(value*LFO_MAX_HZ); break;
    case pIdLfoPitchDepth: text = std::to_string(value); break;
    case pIdLfoOscADepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdLfoOscBDepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdLfoOscMixDepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdLfoFltDepth: text = std::to_string(value*2.0f-1.0f); break;

    case pIdVelAmp: text = std::to_string(value); break;
    case pIdVelOscADepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdVelOscBDepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdVelOscMixDepth: text = std::to_string(value*2.0f-1.0f); break;
    case pIdVelFltCutoff: text = std::to_string(value*2.0f-1.0f); break;
    case pIdVelFltReso: text = std::to_string(value*2.0f-1.0f); break;

    case pIdPolyphony: text = value<.5f?"Mono":"Poly"; break;
    case pIdPortaSpeed: text = std::to_string(value); break;
    case pIdOutputGain: text = std::to_string(20 * log10(value)) + " dB"; break;
    }

    return text;
}
