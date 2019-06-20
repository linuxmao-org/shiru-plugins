#include "ClickyDrumsPlugin.hpp"
#include "ClickyDrumsShared.hpp"
#include "ClickyDrumsPresets.hpp"
#include <random>

ClickyDrumsPlugin::ClickyDrumsPlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count)
{
    loadProgram(0);

    for(unsigned chn=0;chn<SYNTH_CHANNELS;++chn)
    {
        SynthChannel[chn].duration=0;
        SynthChannel[chn].delay=0;
        SynthChannel[chn].output=0;
        SynthChannel[chn].accumulator=0;
        SynthChannel[chn].ptr=0;
        SynthChannel[chn].type=0;
        SynthChannel[chn].volume=1.0f;
    }

    std::minstd_rand prng;
    prng.seed(1);
    for (unsigned i = 0; i < 16384; ++i) Noise[i] = std::uniform_int_distribution<>{0x00, 0xff}(prng);
}

const char *ClickyDrumsPlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *ClickyDrumsPlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *ClickyDrumsPlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *ClickyDrumsPlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *ClickyDrumsPlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t ClickyDrumsPlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t ClickyDrumsPlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void ClickyDrumsPlugin::initParameter(uint32_t index, Parameter &parameter)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, );

    parameter.hints = kParameterIsAutomable;

    parameter.ranges.min = 0.0;
    parameter.ranges.max = 1.0;
    parameter.ranges.def = PresetData[0].values[index];

    ParameterEnumerationValue *pev = nullptr;

    switch (ParameterFirstOfGroup(index)) {
    case pIdDuration1:
        parameter.ranges.max = 250.0;
        parameter.unit = "ms";
        parameter.symbol = "DURA"; parameter.name = "Duration"; break;
        break;
    case pIdMin1:
        parameter.symbol = "PMIN"; parameter.name = "Pulse min"; break;
        break;
    case pIdMax1:
        parameter.symbol = "PMAX"; parameter.name = "Pulse max"; break;
        break;
    case pIdSeed1:
        parameter.symbol = "SEED"; parameter.name = "Rand seed"; break;
        break;
    case pIdType1:
        parameter.hints |= kParameterIsInteger;
        parameter.enumValues.restrictedMode = true;
        parameter.enumValues.count = 2;
        parameter.enumValues.values = pev = new ParameterEnumerationValue[2];
        pev[0] = ParameterEnumerationValue(0.0, "Clicky");
        pev[1] = ParameterEnumerationValue(1.0, "Squeaky");
        parameter.symbol = "TYPE"; parameter.name = "Sound type"; break;
        break;
    case pIdDrumVolume1:
        parameter.ranges.min = 0.01;
        parameter.hints |= kParameterIsLogarithmic;
        parameter.symbol = "VOLU"; parameter.name = "Drum volume"; break;
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

    unsigned n = ParameterNoteNumber(index);
    if ((int)n != -1) {
        parameter.symbol = "n" + String(n + 1) + parameter.symbol;
        parameter.name = String(NoteNames[n]) + " " + parameter.name;
    }
    else
        parameter.symbol = "p" + parameter.symbol;
}

float ClickyDrumsPlugin::getParameterValue(uint32_t index) const
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, 0);

    return Program.values[index];
}

void ClickyDrumsPlugin::setParameterValue(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count,);

    Program.values[index] = value;
}

void ClickyDrumsPlugin::initProgramName(uint32_t index, String &name)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    name = PresetData[index].name;
}

void ClickyDrumsPlugin::loadProgram(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    setState("ProgramName", PresetData[index].name);

    for (unsigned p = 0; p < Parameter_Count; ++p)
        Program.values[p] = PresetData[index].values[p];
}

void ClickyDrumsPlugin::initState(uint32_t index, String &state_key, String &default_value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < State_Count, );

    switch (index) {
    case 0:
        state_key = "ProgramName";
        default_value = "default";
        break;
    }
}

String ClickyDrumsPlugin::getState(const char *key) const
{
    if (!strcmp(key, "ProgramName"))
        return String(Program.name);

    DISTRHO_SAFE_ASSERT_RETURN(false, String());
}

void ClickyDrumsPlugin::setState(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramName"))
        memcpy(Program.name, value, strnlen(value, MAX_NAME_LEN) + 1);
}

void ClickyDrumsPlugin::run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count)
{
    float *outL=outputs[0];
    float *outR=outputs[1];
    float min,max,level,sampleRate;
    unsigned int chn,note;

    sampleRate=getSampleRate();

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
                int pOffset = note * (pIdDuration2 - pIdDuration1);

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
                    if(Program.values[pIdMin1 + pOffset]<Program.values[pIdMax1 + pOffset])
                    {
                        min=Program.values[pIdMin1 + pOffset];
                        max=Program.values[pIdMax1 + pOffset];
                    }
                    else
                    {
                        min=Program.values[pIdMax1 + pOffset];
                        max=Program.values[pIdMin1 + pOffset];
                    }

                    SynthChannel[chn].duration=Program.values[pIdDuration1 + pOffset]*(sampleRate*1e-3f);
                    SynthChannel[chn].accumulator=0;
                    SynthChannel[chn].ptr=(int)(16383.0f*Program.values[pIdSeed1 + pOffset]);
                    SynthChannel[chn].volume=Program.values[pIdDrumVolume1 + pOffset]*((float)msg[2]/100.0f);
                    SynthChannel[chn].delay=0;
                    SynthChannel[chn].delay_min=256.0f*8.0f*min;
                    SynthChannel[chn].delay_max=256.0f*8.0f*max;
                    SynthChannel[chn].type=(int)Program.values[pIdType1 + pOffset];
                    SynthChannel[chn].output=0;
                }
                /*else//key off
                  {
                  SynthChannel[chn].duration=0;
                  }*/
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

        for(chn=0;chn<SYNTH_CHANNELS;++chn)
        {
            if(SynthChannel[chn].duration>0)
            {
                SynthChannel[chn].duration-=1.0f;

                SynthChannel[chn].accumulator+=(65536.0f*4.0f/sampleRate);

                while(SynthChannel[chn].accumulator>=1.0f)
                {
                    SynthChannel[chn].accumulator-=1.0f;

                    SynthChannel[chn].delay-=1.0f;

                    if(SynthChannel[chn].delay<=0)
                    {
                        SynthChannel[chn].delay=(float)Noise[SynthChannel[chn].ptr&16383]*8.0f;

                        if(!SynthChannel[chn].type)
                        {
                            if(SynthChannel[chn].delay<SynthChannel[chn].delay_min) SynthChannel[chn].delay=SynthChannel[chn].delay_min;
                            if(SynthChannel[chn].delay>SynthChannel[chn].delay_max) SynthChannel[chn].delay=SynthChannel[chn].delay_max;
                        }
                        else
                        {
                            while(SynthChannel[chn].delay< SynthChannel[chn].delay_min&&SynthChannel[chn].delay_min>0) SynthChannel[chn].delay+=SynthChannel[chn].delay_min;
                            while(SynthChannel[chn].delay>=SynthChannel[chn].delay_max&&SynthChannel[chn].delay_max>0) SynthChannel[chn].delay-=SynthChannel[chn].delay_max;
                        }

                        ++SynthChannel[chn].ptr;

                        SynthChannel[chn].output^=1;
                    }
                }
            }
            else
            {
                SynthChannel[chn].output=0;
            }

            if(SynthChannel[chn].output) level+=SynthChannel[chn].volume;
        }

        level/=3.0f;//normally there is no more than three drum channels playing at once

        level*=Program.values[pIdOutputGain];

        if(level>1.0f) level=1.0f;

        outL[frame_index]=level;
        outR[frame_index]=level;
    }
}

///
namespace DISTRHO {

Plugin *createPlugin()
{
    return new ClickyDrumsPlugin;
}

} // namespace DISTRHO
