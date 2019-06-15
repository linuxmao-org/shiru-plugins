#include "ChipDrumShared.hpp"

ParameterName GetParameterName(uint32_t index)
{
    ParameterName parameter;

    switch (ParameterFirstOfGroup(index)) {
    case Parameter_ToneLevel1:
        parameter.symbol = "TLVL"; parameter.name = "Tone Level"; break;
    case Parameter_ToneDecay1:
        parameter.symbol = "TDC1"; parameter.name = "Tone Decay Time"; break;
    case Parameter_ToneSustain1:
        parameter.symbol = "TDCL"; parameter.name = "Tone Sustain Level"; break;
    case Parameter_ToneRelease1:
        parameter.symbol = "TDC2"; parameter.name = "Tone Release Time"; break;
    case Parameter_TonePitch1:
        parameter.symbol = "TNPI"; parameter.name = "Tone Pitch"; break;
    case Parameter_ToneSlide1:
        parameter.symbol = "TNSL"; parameter.name = "Tone Slide"; break;
    case Parameter_ToneWave1:
        parameter.symbol = "TWAV"; parameter.name = "Tone Waveform"; break;
    case Parameter_ToneOver1:
        parameter.symbol = "TOVR"; parameter.name = "Tone Overdrive"; break;

    case Parameter_NoiseLevel1:
        parameter.symbol = "LVLN"; parameter.name = "Noise Level"; break;
    case Parameter_NoiseDecay1:
        parameter.symbol = "DC1N"; parameter.name = "Noise Decay Time"; break;
    case Parameter_NoiseSustain1:
        parameter.symbol = "DCLN"; parameter.name = "Noise Sustain Level"; break;
    case Parameter_NoiseRelease1:
        parameter.symbol = "DC2N"; parameter.name = "Noise Release Time"; break;
    case Parameter_NoisePitch11:
        parameter.symbol = "PT1N"; parameter.name = "Noise Main Pitch"; break;
    case Parameter_NoisePitch21:
        parameter.symbol = "PT2N"; parameter.name = "Noise Secondary Pitch"; break;
    case Parameter_NoisePitch2Off1:
        parameter.symbol = "2OFN"; parameter.name = "Noise Secondary Pitch Offset"; break;
    case Parameter_NoisePitch2Len1:
        parameter.symbol = "2LNN"; parameter.name = "Noise Secondary Pitch Duration"; break;
    case Parameter_NoisePeriod1:
        parameter.symbol = "PRDN"; parameter.name = "Noise Period"; break;
    case Parameter_NoiseSeed1:
        parameter.symbol = "PSEN"; parameter.name = "Noise Seed"; break;
    case Parameter_NoiseType1:
        parameter.symbol = "TYPN"; parameter.name = "Noise Type"; break;

    case Parameter_RetrigTime1:
        parameter.symbol = "RTME"; parameter.name = "Retrigger Time"; break;
    case Parameter_RetrigCount1:
        parameter.symbol = "RCNT"; parameter.name = "Retrigger Count"; break;
    case Parameter_RetrigRoute1:
        parameter.symbol = "RRTE"; parameter.name = "Retrigger Route"; break;

    case Parameter_FilterLP1:
        parameter.symbol = "LLPF"; parameter.name = "Filter Low Pass"; break;
    case Parameter_FilterHP1:
        parameter.symbol = "HHPF"; parameter.name = "Filter High Pass"; break;
    case Parameter_FilterRoute1:
        parameter.symbol = "FLTR"; parameter.name = "Filter Route"; break;

    case Parameter_DrumGroup1:
        parameter.symbol = "GRPO"; parameter.name = "Drum Output"; break;
    case Parameter_DrumBitDepth1:
        parameter.symbol = "BDPT"; parameter.name = "Drum Bit Depth"; break;
    case Parameter_DrumUpdateRate1:
        parameter.symbol = "UPDR"; parameter.name = "Drum Update Rate"; break;
    case Parameter_DrumVolume1:
        parameter.symbol = "VOLU"; parameter.name = "Drum Volume"; break;
    case Parameter_DrumPan1:
        parameter.symbol = "PANO"; parameter.name = "Drum Pan"; break;

    case Parameter_VelDrumVolume1:
        parameter.symbol = "VDVL"; parameter.name = "Vel to Volume"; break;

    case Parameter_VelTonePitch1:
        parameter.symbol = "VTPL"; parameter.name = "Vel to Tone Pitch"; break;
    case Parameter_VelNoisePitch1:
        parameter.symbol = "VNPL"; parameter.name = "Vel to Noise Pitch"; break;
    case Parameter_VelToneOver1:
        parameter.symbol = "VODL"; parameter.name = "Vel to Tone Overdrive"; break;

    case Parameter_Hat1Length:
        parameter.symbol = "1LNH"; parameter.name = "Hat F# Length"; break;
    case Parameter_Hat2Length:
        parameter.symbol = "L2NH"; parameter.name = "Hat G# Length"; break;
    case Parameter_Hat3Length:
        parameter.symbol = "LN3H"; parameter.name = "Hat A# Length"; break;
    case Parameter_HatPanWidth:
        parameter.symbol = "HPAN"; parameter.name = "Hat Pan Width"; break;

    case Parameter_Tom1Pitch:
        parameter.symbol = "1PTT"; parameter.name = "Tom G Pitch Offset"; break;
    case Parameter_Tom2Pitch:
        parameter.symbol = "P2TT"; parameter.name = "Tom A Pitch Offset"; break;
    case Parameter_Tom3Pitch:
        parameter.symbol = "PT3T"; parameter.name = "Tom B Pitch Offset"; break;
    case Parameter_TomPanWidth:
        parameter.symbol = "TPAN"; parameter.name = "Tom Pan Width"; break;

    case Parameter_OutputGain:
        parameter.symbol = "GAIN"; parameter.name = "Output Gain"; break;
    }

    unsigned n = ParameterNoteNumber(index);
    if ((int)n != -1) {
        parameter.symbol = std::string("n") + std::to_string(n + 1) + parameter.symbol;
        parameter.name = std::string("Note ") + std::to_string(n + 1) + std::string(" ") + parameter.name;
    }
    else
        parameter.symbol = std::string("p") + parameter.symbol;

    return parameter;
}
