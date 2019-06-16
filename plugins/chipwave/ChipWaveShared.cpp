#include "ChipWaveShared.hpp"
#include "ChipWavePresets.hpp"
#include "DistrhoPlugin.hpp"
#include <random>
#include <cmath>

static std::array<int32_t, 65536> InitNoise()
{
    std::minstd_rand prng;
    prng.seed(1);
    std::array<int32_t, 65536> noise;
    for (unsigned i = 0; i < 65536; ++i) noise[i] = prng() & 1;
    return noise;
}

const std::array<int32_t, 65536> Noise = InitNoise();

float OverdriveValue(float value)
{
    if(value<.5f) return value*2.0f; else return 1.0f+(value-.5f)*2.0f*(OVERDRIVE_MAX-1.0f);
}

float SynthGetSample(SynthOscObject *osc, float over, float duty, int wave)
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

            out=Noise[((osc->noise&noise_mask[mask])+osc->noise_seed)&65535]?.5f:0;

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

void InitParameter(uint32_t index, Parameter &parameter)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, );

    parameter.hints = kParameterIsAutomable;

    parameter.ranges.min = 0.0;
    parameter.ranges.max = 1.0;
    parameter.ranges.def = PresetData[0].values[index];

    switch (index) {
    case pIdOscAWave:
        parameter.symbol = "OAWF"; parameter.name = "OscA Waveform"; break;
    case pIdOscADuty:
        parameter.symbol = "OADU"; parameter.name = "OscA Skew/Duty"; break;
    case pIdOscAOver:
        parameter.symbol = "OAOV"; parameter.name = "OscA Overdrive"; break;
    case pIdOscACut:
        parameter.symbol = "OACT"; parameter.name = "OscA Cut"; break;
    case pIdOscAMultiple:
        parameter.symbol = "OAMU"; parameter.name = "OscA Div/Mul"; break;
    case pIdOscASeed:
        parameter.symbol = "OASD"; parameter.name = "OscA Noise Seed"; break;

    case pIdOscBWave:
        parameter.symbol = "OBWF"; parameter.name = "OscB Waveform"; break;
    case pIdOscBDuty:
        parameter.symbol = "OBDU"; parameter.name = "OscB Skew/Duty"; break;
    case pIdOscBOver:
        parameter.symbol = "OBOV"; parameter.name = "OscB Overdrive"; break;
    case pIdOscBCut:
        parameter.symbol = "OBCT"; parameter.name = "OscB Cut"; break;
    case pIdOscBDetune:
        parameter.symbol = "OBDE"; parameter.name = "OscB Detune"; break;
    case pIdOscBMultiple:
        parameter.symbol = "OBMU"; parameter.name = "OscB Div/Mul"; break;
    case pIdOscBSeed:
        parameter.symbol = "OBSD"; parameter.name = "OscB Noise Seed"; break;

    case pIdOscBalance:
        parameter.symbol = "OBAL"; parameter.name = "Osc Balance"; break;
    case pIdOscMixMode:
        parameter.symbol = "OMIX"; parameter.name = "Osc Mix Mode"; break;

    case pIdFltCutoff:
        parameter.symbol = "FLCU"; parameter.name = "Filter Cutoff"; break;
    case pIdFltReso:
        parameter.symbol = "FLRE"; parameter.name = "Filter Resonance"; break;

    case pIdSlideDelay:
        parameter.symbol = "SLDE"; parameter.name = "Slide Delay"; break;
    case pIdSlideSpeed:
        parameter.symbol = "SLSP"; parameter.name = "Slide Speed"; break;
    case pIdSlideRoute:
        parameter.symbol = "SLRO"; parameter.name = "Slide Route"; break;

    case pIdEnvAttack:
        parameter.symbol = "ENAT"; parameter.name = "Envelope Attack"; break;
    case pIdEnvDecay:
        parameter.symbol = "ENDC"; parameter.name = "Envelope Decay"; break;
    case pIdEnvSustain:
        parameter.symbol = "ENSU"; parameter.name = "Envelope Sustain"; break;
    case pIdEnvRelease:
        parameter.symbol = "ENRE"; parameter.name = "Envelope Release"; break;
    case pIdEnvOscADepth:
        parameter.symbol = "ENOA"; parameter.name = "Env OscA Skew Depth"; break;
    case pIdEnvOscBDepth:
        parameter.symbol = "ENOB"; parameter.name = "Env OscB Skew Depth"; break;
    case pIdEnvOscBDetuneDepth:
        parameter.symbol = "ENOD"; parameter.name = "Env OscB Detune Depth"; break;
    case pIdEnvOscMixDepth:
        parameter.symbol = "ENMX"; parameter.name = "Env Osc Balance Depth"; break;
    case pIdEnvFltDepth:
        parameter.symbol = "ENFL"; parameter.name = "Env Filter Depth"; break;
    case pIdEnvLfoDepth:
        parameter.symbol = "ENLF"; parameter.name = "Env LFO Depth"; break;

    case pIdLfoSpeed:
        parameter.symbol = "LFSP"; parameter.name = "LFO Frequency"; break;
    case pIdLfoPitchDepth:
        parameter.symbol = "LFPI"; parameter.name = "LFO Pitch Depth"; break;
    case pIdLfoOscADepth:
        parameter.symbol = "LFOA"; parameter.name = "LFO OscA Skew Depth"; break;
    case pIdLfoOscBDepth:
        parameter.symbol = "LFOB"; parameter.name = "LFO OscB Skew Depth"; break;
    case pIdLfoOscMixDepth:
        parameter.symbol = "LFMX"; parameter.name = "LFO Osc Balance Depth"; break;
    case pIdLfoFltDepth:
        parameter.symbol = "LFFL"; parameter.name = "LFO Filter Depth"; break;

    case pIdAmpAttack:
        parameter.symbol = "VOAT"; parameter.name = "Amp Attack"; break;
    case pIdAmpDecay:
        parameter.symbol = "VODE"; parameter.name = "Amp Decay"; break;
    case pIdAmpSustain:
        parameter.symbol = "VOSU"; parameter.name = "Amp Sustain"; break;
    case pIdAmpRelease:
        parameter.symbol = "VORE"; parameter.name = "Amp Release"; break;

    case pIdVelAmp:
        parameter.symbol = "VLAM"; parameter.name = "Vel to Amp"; break;
    case pIdVelOscADepth:
        parameter.symbol = "VLOA"; parameter.name = "Vel to OscA Skew Depth"; break;
    case pIdVelOscBDepth:
        parameter.symbol = "VLOB"; parameter.name = "Vel to OscB Skew Depth"; break;
    case pIdVelOscMixDepth:
        parameter.symbol = "VLMX"; parameter.name = "Vel to Osc Balance Depth"; break;
    case pIdVelFltCutoff:
        parameter.symbol = "VLFC"; parameter.name = "Vel to Filter Cutoff"; break;
    case pIdVelFltReso:
        parameter.symbol = "VLFR"; parameter.name = "Vel to Filter Resonance"; break;

    case pIdPolyphony:
        parameter.symbol = "POLY"; parameter.name = "Polyphony"; break;
    case pIdPortaSpeed:
        parameter.symbol = "POSP"; parameter.name = "Porta Speed"; break;
    case pIdOutputGain:
        parameter.symbol = "GAIN"; parameter.name = "Output Gain"; break;
    }
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
