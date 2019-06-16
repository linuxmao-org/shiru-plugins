#include "ChipDrumShared.hpp"

ParameterName GetParameterName(uint32_t index)
{
    ParameterName parameter;

    switch (ParameterFirstOfGroup(index)) {
    case pIdToneLevel1:
        parameter.symbol = "TLVL"; parameter.name = "Tone Level"; break;
    case pIdToneDecay1:
        parameter.symbol = "TDC1"; parameter.name = "Tone Decay Time"; break;
    case pIdToneSustain1:
        parameter.symbol = "TDCL"; parameter.name = "Tone Sustain Level"; break;
    case pIdToneRelease1:
        parameter.symbol = "TDC2"; parameter.name = "Tone Release Time"; break;
    case pIdTonePitch1:
        parameter.symbol = "TNPI"; parameter.name = "Tone Pitch"; break;
    case pIdToneSlide1:
        parameter.symbol = "TNSL"; parameter.name = "Tone Slide"; break;
    case pIdToneWave1:
        parameter.symbol = "TWAV"; parameter.name = "Tone Waveform"; break;
    case pIdToneOver1:
        parameter.symbol = "TOVR"; parameter.name = "Tone Overdrive"; break;

    case pIdNoiseLevel1:
        parameter.symbol = "LVLN"; parameter.name = "Noise Level"; break;
    case pIdNoiseDecay1:
        parameter.symbol = "DC1N"; parameter.name = "Noise Decay Time"; break;
    case pIdNoiseSustain1:
        parameter.symbol = "DCLN"; parameter.name = "Noise Sustain Level"; break;
    case pIdNoiseRelease1:
        parameter.symbol = "DC2N"; parameter.name = "Noise Release Time"; break;
    case pIdNoisePitch11:
        parameter.symbol = "PT1N"; parameter.name = "Noise Main Pitch"; break;
    case pIdNoisePitch21:
        parameter.symbol = "PT2N"; parameter.name = "Noise Secondary Pitch"; break;
    case pIdNoisePitch2Off1:
        parameter.symbol = "2OFN"; parameter.name = "Noise Secondary Pitch Offset"; break;
    case pIdNoisePitch2Len1:
        parameter.symbol = "2LNN"; parameter.name = "Noise Secondary Pitch Duration"; break;
    case pIdNoisePeriod1:
        parameter.symbol = "PRDN"; parameter.name = "Noise Period"; break;
    case pIdNoiseSeed1:
        parameter.symbol = "PSEN"; parameter.name = "Noise Seed"; break;
    case pIdNoiseType1:
        parameter.symbol = "TYPN"; parameter.name = "Noise Type"; break;

    case pIdRetrigTime1:
        parameter.symbol = "RTME"; parameter.name = "Retrigger Time"; break;
    case pIdRetrigCount1:
        parameter.symbol = "RCNT"; parameter.name = "Retrigger Count"; break;
    case pIdRetrigRoute1:
        parameter.symbol = "RRTE"; parameter.name = "Retrigger Route"; break;

    case pIdFilterLP1:
        parameter.symbol = "LLPF"; parameter.name = "Filter Low Pass"; break;
    case pIdFilterHP1:
        parameter.symbol = "HHPF"; parameter.name = "Filter High Pass"; break;
    case pIdFilterRoute1:
        parameter.symbol = "FLTR"; parameter.name = "Filter Route"; break;

    case pIdDrumGroup1:
        parameter.symbol = "GRPO"; parameter.name = "Drum Output"; break;
    case pIdDrumBitDepth1:
        parameter.symbol = "BDPT"; parameter.name = "Drum Bit Depth"; break;
    case pIdDrumUpdateRate1:
        parameter.symbol = "UPDR"; parameter.name = "Drum Update Rate"; break;
    case pIdDrumVolume1:
        parameter.symbol = "VOLU"; parameter.name = "Drum Volume"; break;
    case pIdDrumPan1:
        parameter.symbol = "PANO"; parameter.name = "Drum Pan"; break;

    case pIdVelDrumVolume1:
        parameter.symbol = "VDVL"; parameter.name = "Vel to Volume"; break;

    case pIdVelTonePitch1:
        parameter.symbol = "VTPL"; parameter.name = "Vel to Tone Pitch"; break;
    case pIdVelNoisePitch1:
        parameter.symbol = "VNPL"; parameter.name = "Vel to Noise Pitch"; break;
    case pIdVelToneOver1:
        parameter.symbol = "VODL"; parameter.name = "Vel to Tone Overdrive"; break;

    case pIdHat1Length:
        parameter.symbol = "1LNH"; parameter.name = "Hat F# Length"; break;
    case pIdHat2Length:
        parameter.symbol = "L2NH"; parameter.name = "Hat G# Length"; break;
    case pIdHat3Length:
        parameter.symbol = "LN3H"; parameter.name = "Hat A# Length"; break;
    case pIdHatPanWidth:
        parameter.symbol = "HPAN"; parameter.name = "Hat Pan Width"; break;

    case pIdTom1Pitch:
        parameter.symbol = "1PTT"; parameter.name = "Tom G Pitch Offset"; break;
    case pIdTom2Pitch:
        parameter.symbol = "P2TT"; parameter.name = "Tom A Pitch Offset"; break;
    case pIdTom3Pitch:
        parameter.symbol = "PT3T"; parameter.name = "Tom B Pitch Offset"; break;
    case pIdTomPanWidth:
        parameter.symbol = "TPAN"; parameter.name = "Tom Pan Width"; break;

    case pIdOutputGain:
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
