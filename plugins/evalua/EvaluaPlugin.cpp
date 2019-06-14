#include "EvaluaPlugin.hpp"
#include "ev.h"
#include <cstdio>
#include <cstring>

EvaluaPlugin::EvaluaPlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count),
      ev(new EV)
{
    memset(&Program, 0, sizeof(Program));

    loadProgram(0);

    for (unsigned chn = 0; chn < MaxPolyphony; ++chn) {
        SynthChannel[chn].note = -1;
        SynthChannel[chn].freq = 0;
        SynthChannel[chn].freq_new = 0;
    }

    Compile();

    Polyphony = PresetPolyphony;
    PortaSpeed = PresetPortaSpeed;
    OutputGain = PresetOuputGain;

    MidiPitchBend = 0;
    MidiPitchBendRange = 2.0f;
    MidiModulationDepth = 0;

    MidiRPNLSB = 0;
    MidiRPNMSB = 0;
    MidiDataLSB = 0;
    MidiDataMSB = 0;

    memset(MidiKeyState, 0, sizeof(MidiKeyState));

    SlideStep = 0;
}

const char *EvaluaPlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *EvaluaPlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *EvaluaPlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *EvaluaPlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *EvaluaPlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t EvaluaPlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t EvaluaPlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void EvaluaPlugin::initParameter(uint32_t index, Parameter &parameter)
{
    (void)index;
    (void)parameter;
}

float EvaluaPlugin::getParameterValue(uint32_t index) const
{
    (void)index;
    return 0;
}

void EvaluaPlugin::setParameterValue(uint32_t index, float value)
{
    (void)index;
    (void)value;
}

void EvaluaPlugin::initProgramName(uint32_t index, String &name)
{
    if (index < PresetDataLength)
        name = PresetData[index].first;
    else {
        char namebuf[32];
        sprintf(namebuf, "%3.3u default", index);
        name = namebuf;
    }
}

void EvaluaPlugin::loadProgram(uint32_t index)
{
    if (index < PresetDataLength) {
        strncpy(Program.Name, PresetData[index].first, MAX_NAME_LEN);
        strncpy(Program.Data, PresetData[index].second, MAX_PROGRAM_LEN);
    }
    else {
        sprintf(Program.Name, "%3.3u default", index);
        strncpy(Program.Data, PresetData[0].second, MAX_PROGRAM_LEN);
    }

    Polyphony = PresetPolyphony;
    PortaSpeed = PresetPortaSpeed;
    OutputGain = PresetOuputGain;

    Compile();
}

void EvaluaPlugin::initState(uint32_t index, String &state_key, String &default_value)
{
    switch (index) {
    case State_ProgramName:
        state_key = "ProgramName";
        default_value = "Default";
        break;
    case State_ProgramData:
        state_key = "ProgramData";
        default_value = PresetData[0].second;
        break;
    case State_Polyphony:
        state_key = "Polyphony";
        default_value = String(PresetPolyphony);
        break;
    case State_PortaSpeed:
        state_key = "PortaSpeed";
        default_value = String(PresetPortaSpeed);
        break;
    case State_OutputGain:
        state_key = "OutputGain";
        default_value = String(PresetOuputGain);
        break;
    }
}

///
static int stringToIntBounded(const char *value, int min, int max)
{
    long val = strtol(value, nullptr, 0);
    val = (val < min) ? min : val;
    val = (val > max) ? max : val;
    return (int)val;
}

///
String EvaluaPlugin::getState(const char *key) const
{
    if (!strcmp(key, "ProgramName"))
        return String(Program.Name);
    else if (!strcmp(key, "ProgramData"))
        return String(Program.Data);
    else if (!strcmp(key, "Polyphony"))
        return String(Polyphony);
    else if (!strcmp(key, "PortaSpeed"))
        return String(PortaSpeed);
    else if (!strcmp(key, "OutputGain"))
        return String(OutputGain);
    else
        return String();
}

void EvaluaPlugin::setState(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramData")) {
        strncpy(Program.Data, value, MAX_PROGRAM_LEN);
        Compile();
    }
    else if (!strcmp(key, "ProgramName"))
        strncpy(Program.Name, value, MAX_NAME_LEN);
    else if (!strcmp(key, "Polyphony"))
        Polyphony = stringToIntBounded(value, MinPolyphony, MaxPolyphony);
    else if (!strcmp(key, "PortaSpeed"))
        PortaSpeed = stringToIntBounded(value, MinPortaSpeed, MaxPortaSpeed);
    else if (!strcmp(key, "OutputGain"))
        OutputGain = stringToIntBounded(value, MinOutputGain, MaxOutputGain);
}

void EvaluaPlugin::run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count)
{
    ChannelObject *ch;
    float *outL = outputs[0];
    float *outR = outputs[1];
    float output, level;
    double freq, sample_rate, time_delta, pitch_delta;
    int32_t chn;
    int64_t n;

    std::minstd_rand &rng = RandomNumberGenerator;

    sample_rate = getSampleRate();

    time_delta = 65536.0 / sample_rate;

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
            case 0x80: // note off
            case 0x90: // note on
                if(msg[0] == 0x90 && msg[2] > 0)//key on
                {
                    MidiKeyState[msg[1]] = 1;

                    chn = SynthAllocateVoice(msg[0] & 0xf, msg[1]);

                    if(chn>=0)
                    {
                        SynthChannelChangeNote(chn, msg[0] & 0xf, msg[1], msg[2]);
                    }
                }
                else//key off
                {
                    MidiKeyState[msg[1]] = 0;

                    SynthChannelReleaseNote(msg[0] & 0xf, msg[1]);

                    if (Polyphony > 1)
                    {
                        for (chn = 0; chn < MaxPolyphony; ++chn) if (SynthChannel[chn].note == msg[1]) SynthChannelReleaseNote(msg[0] & 0xf, msg[1]);
                    }
                    else
                    {
                        for (int32_t note = 127; note >= 0; --note)
                        {
                            if (MidiKeyState[note])
                            {
                                SynthChannelChangeNote(0, msg[0] & 0xf, note, -1);

                                break;
                            }
                        }
                    }
                }
                break;

            case 0xb0: //control change
                switch (msg[1]) {
                case 0x64: MidiRPNLSB = msg[2]; break;
                case 0x65: MidiRPNMSB = msg[2]; break;
                case 0x26: MidiDataLSB = msg[2]; break;

                case 0x01: MidiModulationDepth = (int64_t)msg[2]; break;

                case 0x06:
                    MidiDataMSB = msg[2];

                    if (MidiRPNLSB == 0 && MidiRPNMSB == 0)
                        MidiPitchBendRange = (float)MidiDataMSB * .5f;

                    break;

                case 0x7b: //all notes off and mono/poly mode changes that also requires to do all notes off
                    memset(MidiKeyState,0,sizeof(MidiKeyState));    //stop all channels

                    SynthChannelReleaseNote(-1,-1);
                    break;
                }
                break;

#if 0
            case 0xc0: //program change
                if (ProgramIndex != msg[1])
                {
                    ProgramIndex = msg[1];

                    Compile();

                    SetUpdateGUI();
                }
                break;
#endif

            case 0xe0:  //pitch bend change
            {
                int32_t wheel = msg[1] | (msg[2] << 7);
                MidiPitchBend = (float)((wheel - 0x2000) * MidiPitchBendRange / 8192.0);
                break;
            }
            }
        }

        //generate sound, update channels if needed

        ev->ClearVars();

        level = 0;

        for (chn = 0; chn < MaxPolyphony; ++chn)
        {
            ch = &SynthChannel[chn];

            if (ch->note < 0) continue;

            if (PortaSpeed >= MaxPortaSpeed)
            {
                ch->freq = ch->freq_new;
            }
            else
            {
                if (ch->freq < ch->freq_new)
                {
                    ch->freq += SlideStep / sample_rate;

                    if (ch->freq > ch->freq_new) ch->freq = ch->freq_new;
                }

                if (ch->freq > ch->freq_new)
                {
                    ch->freq += SlideStep / sample_rate;

                    if (ch->freq < ch->freq_new) ch->freq = ch->freq_new;
                }
            }

            freq = ch->freq + MidiPitchBend;

            pitch_delta = 440.0*pow(2.0, freq / 12.0) * 256.0 / sample_rate;    //256 considered a full waveform period

            ch->pitch_acc += pitch_delta;

            while (ch->pitch_acc >= 1.0)
            {
                ch->pitch_acc -= 1.0;
                ch->pitch_cnt += 1;
            }

            ch->time_acc += time_delta;

            while (ch->time_acc >= 1.0)
            {
                ch->time_acc -= 1.0;
                ch->time_cnt += 1;
            }

            ev->SetVar('T', ch->time_cnt);
            ev->SetVar('P', ch->pitch_cnt);
            ev->SetVar('V', ch->velocity);
            ev->SetVar('M', MidiModulationDepth);
            ev->SetVar('R', std::uniform_int_distribution<>{0, 32767}(rng));

            n = ev->Solve();

            if (n < -256) n = -256;
            if (n > 256) n = 256;

            n = n * OutputGain / NormalOutputGain;

            output = (float)n / 256.0f;

            level += output;
        }

        level = level * .25f;

        if (level < -1.0f) level = -1.0f;
        if (level > 1.0f) level = 1.0f;

        (*outL++)=level;
        (*outR++)=level;
    }

    //UpdateGUI();
}

const char *EvaluaPlugin::Compile()
{
    return ev->Parse(Program.Data);
}

int32_t EvaluaPlugin::SynthAllocateVoice(int32_t midi_ch, int32_t note)
{
    int32_t chn;

    if (Polyphony == 1) return 0;  //always use ch0 in mono mode

    for (chn = 0; chn < Polyphony; ++chn) if (SynthChannel[chn].note == note) return chn;
    for (chn = 0; chn < Polyphony; ++chn) if (SynthChannel[chn].note < 0) return chn;

    return -1;
}

void EvaluaPlugin::SynthChannelChangeNote(int32_t chn, int32_t midi_ch, int32_t note, int32_t velocity)
{
    ChannelObject *ch;
    int32_t prev_note;

    ch = &SynthChannel[chn];

    prev_note = ch->note;

    ch->note = note;

    if (note >= 0)
    {
        ch->freq_new = ch->note - 69;

        if ((velocity >= 0 && prev_note < 0) || Polyphony > 1) ch->freq = ch->freq_new;

        SlideStep = (ch->freq - ch->freq_new)*20.0*log(1.0 - ((double)PortaSpeed / 100.0));
    }

    if (velocity >= 0)
    {
        ch->velocity = velocity;

        ch->time_cnt = 0;
        ch->pitch_cnt = 0;
        ch->time_acc = 0;
        ch->pitch_acc = 0;
    }
}

void EvaluaPlugin::SynthChannelReleaseNote(int32_t midi_ch, int32_t note)
{
    if (note >= 0) {
        for (unsigned chn = 0; chn < MaxPolyphony; ++chn) {
            if (SynthChannel[chn].note == note) SynthChannel[chn].note = -1;
        }
    }
    else
    {
        for (unsigned chn = 0; chn < MaxPolyphony; ++chn)
        {
            SynthChannel[chn].note = -1;
        }
    }
}

namespace DISTRHO {

Plugin *createPlugin()
{
    return new EvaluaPlugin;
}

} // namespace DISTRHO
