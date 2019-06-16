#include "DistrhoPluginInfo.h"

ParameterName GetParameterName(uint32_t index)
{
    ParameterName parameter;

    parameter.symbol = nullptr;
    parameter.name = nullptr;

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

    return parameter;
}
