#include "DistrhoPluginInfo.h"

ParameterName GetParameterName(uint32_t index)
{
    ParameterName parameter;

    parameter.symbol = nullptr;
    parameter.name = nullptr;

    switch (index) {
    case Parameter_OscAWave:
        parameter.symbol = "OAWF"; parameter.name = "OscA Waveform"; break;
    case Parameter_OscADuty:
        parameter.symbol = "OADU"; parameter.name = "OscA Skew/Duty"; break;
    case Parameter_OscAOver:
        parameter.symbol = "OAOV"; parameter.name = "OscA Overdrive"; break;
    case Parameter_OscACut:
        parameter.symbol = "OACT"; parameter.name = "OscA Cut"; break;
    case Parameter_OscAMultiple:
        parameter.symbol = "OAMU"; parameter.name = "OscA Div/Mul"; break;
    case Parameter_OscASeed:
        parameter.symbol = "OASD"; parameter.name = "OscA Noise Seed"; break;

    case Parameter_OscBWave:
        parameter.symbol = "OBWF"; parameter.name = "OscB Waveform"; break;
    case Parameter_OscBDuty:
        parameter.symbol = "OBDU"; parameter.name = "OscB Skew/Duty"; break;
    case Parameter_OscBOver:
        parameter.symbol = "OBOV"; parameter.name = "OscB Overdrive"; break;
    case Parameter_OscBCut:
        parameter.symbol = "OBCT"; parameter.name = "OscB Cut"; break;
    case Parameter_OscBDetune:
        parameter.symbol = "OBDE"; parameter.name = "OscB Detune"; break;
    case Parameter_OscBMultiple:
        parameter.symbol = "OBMU"; parameter.name = "OscB Div/Mul"; break;
    case Parameter_OscBSeed:
        parameter.symbol = "OBSD"; parameter.name = "OscB Noise Seed"; break;

    case Parameter_OscBalance:
        parameter.symbol = "OBAL"; parameter.name = "Osc Balance"; break;
    case Parameter_OscMixMode:
        parameter.symbol = "OMIX"; parameter.name = "Osc Mix Mode"; break;

    case Parameter_FltCutoff:
        parameter.symbol = "FLCU"; parameter.name = "Filter Cutoff"; break;
    case Parameter_FltReso:
        parameter.symbol = "FLRE"; parameter.name = "Filter Resonance"; break;

    case Parameter_SlideDelay:
        parameter.symbol = "SLDE"; parameter.name = "Slide Delay"; break;
    case Parameter_SlideSpeed:
        parameter.symbol = "SLSP"; parameter.name = "Slide Speed"; break;
    case Parameter_SlideRoute:
        parameter.symbol = "SLRO"; parameter.name = "Slide Route"; break;

    case Parameter_EnvAttack:
        parameter.symbol = "ENAT"; parameter.name = "Envelope Attack"; break;
    case Parameter_EnvDecay:
        parameter.symbol = "ENDC"; parameter.name = "Envelope Decay"; break;
    case Parameter_EnvSustain:
        parameter.symbol = "ENSU"; parameter.name = "Envelope Sustain"; break;
    case Parameter_EnvRelease:
        parameter.symbol = "ENRE"; parameter.name = "Envelope Release"; break;
    case Parameter_EnvOscADepth:
        parameter.symbol = "ENOA"; parameter.name = "Env OscA Skew Depth"; break;
    case Parameter_EnvOscBDepth:
        parameter.symbol = "ENOB"; parameter.name = "Env OscB Skew Depth"; break;
    case Parameter_EnvOscBDetuneDepth:
        parameter.symbol = "ENOD"; parameter.name = "Env OscB Detune Depth"; break;
    case Parameter_EnvOscMixDepth:
        parameter.symbol = "ENMX"; parameter.name = "Env Osc Balance Depth"; break;
    case Parameter_EnvFltDepth:
        parameter.symbol = "ENFL"; parameter.name = "Env Filter Depth"; break;
    case Parameter_EnvLfoDepth:
        parameter.symbol = "ENLF"; parameter.name = "Env LFO Depth"; break;

    case Parameter_LfoSpeed:
        parameter.symbol = "LFSP"; parameter.name = "LFO Frequency"; break;
    case Parameter_LfoPitchDepth:
        parameter.symbol = "LFPI"; parameter.name = "LFO Pitch Depth"; break;
    case Parameter_LfoOscADepth:
        parameter.symbol = "LFOA"; parameter.name = "LFO OscA Skew Depth"; break;
    case Parameter_LfoOscBDepth:
        parameter.symbol = "LFOB"; parameter.name = "LFO OscB Skew Depth"; break;
    case Parameter_LfoOscMixDepth:
        parameter.symbol = "LFMX"; parameter.name = "LFO Osc Balance Depth"; break;
    case Parameter_LfoFltDepth:
        parameter.symbol = "LFFL"; parameter.name = "LFO Filter Depth"; break;

    case Parameter_AmpAttack:
        parameter.symbol = "VOAT"; parameter.name = "Amp Attack"; break;
    case Parameter_AmpDecay:
        parameter.symbol = "VODE"; parameter.name = "Amp Decay"; break;
    case Parameter_AmpSustain:
        parameter.symbol = "VOSU"; parameter.name = "Amp Sustain"; break;
    case Parameter_AmpRelease:
        parameter.symbol = "VORE"; parameter.name = "Amp Release"; break;

    case Parameter_VelAmp:
        parameter.symbol = "VLAM"; parameter.name = "Vel to Amp"; break;
    case Parameter_VelOscADepth:
        parameter.symbol = "VLOA"; parameter.name = "Vel to OscA Skew Depth"; break;
    case Parameter_VelOscBDepth:
        parameter.symbol = "VLOB"; parameter.name = "Vel to OscB Skew Depth"; break;
    case Parameter_VelOscMixDepth:
        parameter.symbol = "VLMX"; parameter.name = "Vel to Osc Balance Depth"; break;
    case Parameter_VelFltCutoff:
        parameter.symbol = "VLFC"; parameter.name = "Vel to Filter Cutoff"; break;
    case Parameter_VelFltReso:
        parameter.symbol = "VLFR"; parameter.name = "Vel to Filter Resonance"; break;

    case Parameter_Polyphony:
        parameter.symbol = "POLY"; parameter.name = "Polyphony"; break;
    case Parameter_PortaSpeed:
        parameter.symbol = "POSP"; parameter.name = "Porta Speed"; break;
    case Parameter_OutputGain:
        parameter.symbol = "GAIN"; parameter.name = "Output Gain"; break;
    }

    return parameter;
}
