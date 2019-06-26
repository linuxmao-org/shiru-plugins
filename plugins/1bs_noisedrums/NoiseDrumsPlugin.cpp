#include "NoiseDrumsPlugin.hpp"
#include "NoiseDrumsShared.hpp"
#include "NoiseDrumsPresets.hpp"
#include <random>

NoiseDrumsPlugin::NoiseDrumsPlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count)
{
    loadProgram(0);

    for(unsigned chn=0;chn<SYNTH_CHANNELS;++chn)
    {
        SynthChannel[chn].burst      =NOISE_MAX_BURSTS;
        SynthChannel[chn].note       =0;
        SynthChannel[chn].duration   =0;
        SynthChannel[chn].accumulator=0;
        SynthChannel[chn].increment  =0;
        SynthChannel[chn].period     =0;
        SynthChannel[chn].ptr        =0;
        SynthChannel[chn].volume     =1.0f;
        SynthChannel[chn].output     =0;
    }

    std::minstd_rand prng;
    prng.seed(1);

    for(unsigned i=0;i<2048;++i)
    {
        unsigned n=0;

        for(unsigned j=0;j<4;++j) n=(n<<8)|(std::uniform_int_distribution<>{0x00, 0xff}(prng));

        Noise[i]=n;
    }
}

const char *NoiseDrumsPlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *NoiseDrumsPlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *NoiseDrumsPlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *NoiseDrumsPlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *NoiseDrumsPlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t NoiseDrumsPlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t NoiseDrumsPlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void NoiseDrumsPlugin::initParameter(uint32_t index, Parameter &parameter)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, );

    parameter.hints = kParameterIsAutomable;

    parameter.ranges.min = 0.0;
    parameter.ranges.max = 1.0;
    parameter.ranges.def = PresetData[0].values[index];

    //ParameterEnumerationValue *pev = nullptr;

    switch (ParameterFirstOfGroup(index)) {
    case pIdDrumVolume1:
        parameter.symbol = "VOLU"; parameter.name = "Drum volume"; break;
        break;
    case pIdBurst1Duration1:
        parameter.symbol = "DURA"; parameter.name = "Duration"; break;
        break;
    case pIdBurst1Pitch1:
        parameter.symbol = "PTCH"; parameter.name = "Pitch"; break;
        break;
    case pIdBurst1Period1:
        parameter.symbol = "PERD"; parameter.name = "Period"; break;
        break;
    case pIdBurst1PulseWidth1:
        parameter.symbol = "PWDT"; parameter.name = "Pulse width"; break;
        break;
    case pIdPolyphony:
        parameter.hints |= kParameterIsInteger|kParameterIsBoolean;
        parameter.symbol = "POLY"; parameter.name = "Polyphony"; break;
        break;
    case pIdOutputGain:
        parameter.ranges.min = 0.01;
        parameter.hints |= kParameterIsLogarithmic;
        parameter.symbol = "GAIN"; parameter.name = "Output gain"; break;
        break;
    default:
        DISTRHO_SAFE_ASSERT(false);
    }

    unsigned nn = ParameterNoteNumber(index);
    unsigned bn = ParameterBurstNumber(index);
    if ((int)nn != -1) {
        if ((int)bn != -1) {
            parameter.symbol = "b" + String(bn + 1) + parameter.symbol;
            parameter.name = "Burst " + String(bn + 1) + " " + parameter.name;
        }
        parameter.symbol = "n" + String(nn + 1) + parameter.symbol;
        parameter.name = String(NoteNames[nn]) + " " + parameter.name;
    }
    else
        parameter.symbol = "p" + parameter.symbol;
}

float NoiseDrumsPlugin::getParameterValue(uint32_t index) const
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, 0);

    return Program.values[index];
}

void NoiseDrumsPlugin::setParameterValue(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count,);

    Program.values[index] = value;
}

void NoiseDrumsPlugin::initProgramName(uint32_t index, String &name)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    name = PresetData[index].name;
}

void NoiseDrumsPlugin::loadProgram(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    setState("ProgramName", PresetData[index].name);

    for (unsigned p = 0; p < Parameter_Count; ++p)
        Program.values[p] = PresetData[index].values[p];
}

void NoiseDrumsPlugin::initState(uint32_t index, String &state_key, String &default_value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < State_Count, );

    switch (index) {
    case 0:
        state_key = "ProgramName";
        default_value = "default";
        break;
    }
}

String NoiseDrumsPlugin::getState(const char *key) const
{
    if (!strcmp(key, "ProgramName"))
        return String(Program.name);

    DISTRHO_SAFE_ASSERT_RETURN(false, String());
}

void NoiseDrumsPlugin::setState(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramName"))
        memcpy(Program.name, value, strnlen(value, MAX_NAME_LEN) + 1);
}

void NoiseDrumsPlugin::run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count)
{
    float *outL=outputs[0];
    float *outR=outputs[1];
    float level;
    unsigned int s,chn,note;

    uint32_t event_index = 0;

    for (uint32_t frame_index = 0; frame_index < frames; ++frame_index)
    {
        while (event_index < event_count)
        {
            const MidiEvent &event = events[event_index];

            if (event.frame > frame_index && frame_index + 1 != frames)
                break;

            ++event_index;

            if (event.size > 4)
                continue;

            uint8_t msg[4] = {};
            for (uint32_t i = 0, n = event.size; i < n; ++i)
                msg[i] = event.data[i] & ((i == 0) ? 0xff : 0x7f);

            //uint8_t channel = msg[0] & 0x0f;
            msg[0] = msg[0] & 0xf0;

            switch(msg[0])
            {
            case 0x90:
            {
                note=msg[1]%12;
                int pNoteOffset = note * (pIdDrumVolume2 - pIdDrumVolume1);

                chn = 0;

                if(Program.values[pIdPolyphony]>.5f)
                {
                    switch(note)
                    {
                    case 0:  chn=0; break;//C
                    case 1:  chn=7; break;//C#
                    case 2:  chn=1; break;//D
                    case 3:  chn=7; break;//D#
                    case 4:  chn=2; break;//E
                    case 5:  chn=3; break;//F
                    case 6:  chn=7; break;//F#
                    case 7:  chn=4; break;//G
                    case 8:  chn=7; break;//G#
                    case 9:  chn=5; break;//A
                    case 10: chn=7; break;//A#
                    case 11: chn=6; break;//B
                    }
                }

                if(msg[0] == 0x90 && msg[2] > 0)//key on
                {
                    SynthChannel[chn].burst=0;
                    SynthChannel[chn].note=note;
                    SynthChannel[chn].accumulator=0;
                    SynthChannel[chn].ptr=0;
                    SynthChannel[chn].volume=Program.values[pIdDrumVolume1 + pNoteOffset]*((float)msg[2]/100.0f);

                    SynthNewNoiseBurst(chn);
                }
            }
            break;

#if 0
            case 0xc0:
            {
                Program=msg[1];
                updateDisplay();
            }
            break;
#endif
            }
        }

        level=0;

        for(s=0;s<OVERSAMPLING;++s)
        {
            for(chn=0;chn<SYNTH_CHANNELS;++chn)
            {
                if(SynthChannel[chn].burst>=NOISE_MAX_BURSTS) continue;

                SynthChannel[chn].duration-=1.0f/OVERSAMPLING;

                SynthChannel[chn].accumulator+=SynthChannel[chn].increment/OVERSAMPLING;

                while(SynthChannel[chn].accumulator>=1.0f)
                {
                    SynthChannel[chn].accumulator-=1.0f;

                    SynthChannel[chn].output=(Noise[(SynthChannel[chn].ptr>>5)&SynthChannel[chn].period]>>(SynthChannel[chn].ptr&31))&1;

                    ++SynthChannel[chn].ptr;
                }

                int pNoteOffset = SynthChannel[chn].note * (pIdDrumVolume2 - pIdDrumVolume1);
                int pBurstOffset = SynthChannel[chn].burst * (pIdBurst2Duration1 - pIdBurst1Duration1);

                if(SynthChannel[chn].accumulator>Program.values[pIdBurst1PulseWidth1 + pNoteOffset + pBurstOffset]) SynthChannel[chn].output=0;

                if(SynthChannel[chn].output) level+=SynthChannel[chn].volume/OVERSAMPLING;

                if(SynthChannel[chn].duration<=0)
                {
                    if(SynthChannel[chn].burst<NOISE_MAX_BURSTS)
                    {
                        ++SynthChannel[chn].burst;

                        SynthNewNoiseBurst(chn);
                    }
                    else
                    {
                        SynthChannel[chn].output=0;
                    }
                }
            }
        }

        level/=3.0f;//it is unusual to have more than three drum channels playing at once

        level*=Program.values[pIdOutputGain];

        if(level>1.0f) level=1.0f;

        outL[frame_index]=level;
        outR[frame_index]=level;
    }
}

int NoiseDrumsPlugin::FloatToNoisePeriod(float value)
{
    const int period[]={4,8,16,32,64,128,256,512,1024,2048};

    return period[(int)(value*9.99f)];
}

void NoiseDrumsPlugin::SynthNewNoiseBurst(int32_t chn)
{
    float sampleRate;

    sampleRate=(float)getSampleRate();

    int pNoteOffset = SynthChannel[chn].note * (pIdDrumVolume2 - pIdDrumVolume1);
    int pBurstOffset = SynthChannel[chn].burst * (pIdBurst2Duration1 - pIdBurst1Duration1);

    SynthChannel[chn].duration =Program.values[pIdBurst1Duration1 + pNoteOffset + pBurstOffset]*NOISE_MAX_DURATION*(sampleRate/1000.0f);
    SynthChannel[chn].increment=Program.values[pIdBurst1Pitch1 + pNoteOffset + pBurstOffset]*NOISE_MAX_PITCH/sampleRate;

    SynthChannel[chn].period=FloatToNoisePeriod(Program.values[pIdBurst1Period1 + pNoteOffset + pBurstOffset])-1;
}

///
namespace DISTRHO {

Plugin *createPlugin()
{
    return new NoiseDrumsPlugin;
}

} // namespace DISTRHO
