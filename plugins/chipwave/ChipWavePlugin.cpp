#include "ChipWavePlugin.hpp"
#include <cstring>
#include <cmath>

enum {
    OVERSAMPLING = 8,
};

ChipWavePlugin::ChipWavePlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count)
{
    Program = PresetData[0];

    memset(SynthChannel,0,sizeof(SynthChannel));

    for(unsigned chn = 0; chn < MaxSynthChannels; ++chn)
    {
        SynthChannel[chn].note = -1;

        SynthChannel[chn].ev_stage = eStageReset;
        SynthChannel[chn].ef_stage = eStageReset;

        SynthChannel[chn].volume = 1.0f;
    }

    RandomNumberGenerator.seed(1);

    sEnvelopeDiv = 0;

    MidiRPNLSB = 0;
    MidiRPNMSB = 0;
    MidiDataLSB = 0;
    MidiDataMSB = 0;

    MidiPitchBend = 0;
    MidiPitchBendRange = 2.0f;
    MidiModulationDepth = 0;
    MidiModulationCount = 0;

    sSlideStep = 0;

    memset(MidiKeyState, 0, sizeof(MidiKeyState));
}

const char *ChipWavePlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *ChipWavePlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *ChipWavePlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *ChipWavePlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *ChipWavePlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t ChipWavePlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t ChipWavePlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void ChipWavePlugin::initParameter(uint32_t index, Parameter &parameter)
{
    InitParameter(index, parameter);
}

float ChipWavePlugin::getParameterValue(uint32_t index) const
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, 0);

    return Program.values[index];
}

void ChipWavePlugin::setParameterValue(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count,);

    Program.values[index] = value;
}

void ChipWavePlugin::initProgramName(uint32_t index, String &name)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    name = PresetData[index].name;
}

void ChipWavePlugin::loadProgram(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    Program = PresetData[index];
}

void ChipWavePlugin::initState(uint32_t index, String &state_key, String &default_value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < State_Count, );

    switch (index) {
    case 0:
        state_key = "ProgramName";
        default_value = "default";
        break;
    }
}

String ChipWavePlugin::getState(const char *key) const
{
    if (!strcmp(key, "ProgramName"))
        return String(Program.name);

    DISTRHO_SAFE_ASSERT_RETURN(false, String());
}

void ChipWavePlugin::setState(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramName")) {
        memcpy(Program.name, value, strnlen(value, MaxNameLen));
        Program.name[MaxNameLen] = '\0';
    }
}

void ChipWavePlugin::run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count)
{
    float *outL=outputs[0];
    float *outR=outputs[1];
    float level_mix,level_osc;
    float modulation,vibrato,mod_step,depth,sampleRate;
    float osca,oscb,dutya,dutyb,osc_balance,detune,overa,overb;
    double q,w;
    unsigned int s;
    int chn,note,prev_note;
    int oscaw,oscbw,mix_mode,slide,osc_mute;
    bool poly,key_off;

    std::minstd_rand &rng = RandomNumberGenerator;

    const float pi = M_PI;

    sampleRate= getSampleRate();

    mod_step=12.0f/sampleRate*pi;

    poly=(Program.values[pIdPolyphony]<.5f?false:true);

    oscaw=(int)(Program.values[pIdOscAWave]*3.99f);
    oscbw=(int)(Program.values[pIdOscBWave]*3.99f);

    overa=OverdriveValue(Program.values[pIdOscAOver]);
    overb=OverdriveValue(Program.values[pIdOscBOver]);

    mix_mode=(int)(Program.values[pIdOscMixMode]<.5f?0:1);

    slide=(int)(Program.values[pIdSlideRoute]*2.99f);

    uint32_t event_index = 0;

    for (uint32_t frame_index = 0; frame_index < frames; ++frame_index)
    {
        if(MidiModulationDepth>=.01f) modulation=sinf(MidiModulationCount)*MidiModulationDepth; else modulation=0;

        MidiModulationCount+=mod_step;

        key_off=false;

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

            switch (msg[0])
            {
            case 0x80: // note off
            case 0x90: // note on
            {
                if(msg[0] == 0x90 && msg[2] > 0)//key on
                {
                    MidiKeyState[msg[1]]=1;

                    if(!poly) chn=0; else chn=SynthAllocateVoice(msg[1]);

                    if(chn>=0)
                    {
                        prev_note=SynthChannel[chn].note;

                        SynthChannelChangeNote(chn,msg[1]);

                        if(prev_note<0||poly) SynthChannel[chn].freq=SynthChannel[chn].freq_new;

                        if(poly||prev_note<0||(!poly&&SynthChannel[0].ev_stage==eStageRelease))
                        {
                            SynthChannel[chn].osca.acc=0;    //phase reset
                            SynthChannel[chn].oscb.acc=0;

                            SynthChannel[chn].osca.cut=sampleRate*(Program.values[pIdOscACut]*OSC_CUT_MAX_MS)/1000.0f;
                            SynthChannel[chn].oscb.cut=sampleRate*(Program.values[pIdOscBCut]*OSC_CUT_MAX_MS)/1000.0f;

                            if(Program.values[pIdOscASeed]>0)
                            {
                                SynthChannel[chn].osca.noise_seed=(int32_t)(Program.values[pIdOscASeed]*65535.99f);
                            }
                            else
                            {
                                SynthChannel[chn].osca.noise_seed=std::uniform_int_distribution<>{0, 32767}(rng);
                            }

                            if(Program.values[pIdOscBSeed]>0)
                            {
                                SynthChannel[chn].oscb.noise_seed=(int32_t)(Program.values[pIdOscBSeed]*65535.99f);
                            }
                            else
                            {
                                SynthChannel[chn].oscb.noise_seed=std::uniform_int_distribution<>{0, 32767}(rng);
                            }

                            SynthChannel[chn].velocity=((float)msg[2]/100.0f);
                            SynthChannel[chn].volume=1.0f*(1.0f-Program.values[pIdVelAmp])+SynthChannel[chn].velocity*Program.values[pIdVelAmp];

                            SynthChannel[chn].slide_delay=sampleRate*(Program.values[pIdSlideDelay]*MOD_DELAY_MAX_MS)/1000.0f;
                            SynthChannel[chn].slide_osca=0;
                            SynthChannel[chn].slide_oscb=0;

                            SynthChannel[chn].lfo_count=0;
                            SynthChannel[chn].lfo_out=0;

                            SynthChannel[chn].filter_vibrapos=0;
                            SynthChannel[chn].filter_vibraspeed=0;

                            SynthRestartEnvelope(chn);
                        }
                    }
                }
                else//key off
                {
                    key_off=true;

                    MidiKeyState[msg[1]]=0;

                    if(poly)
                    {
                        for(chn=0;chn<MaxSynthChannels;++chn) if(SynthChannel[chn].note==msg[1]) SynthStopEnvelope(chn);
                    }
                    else
                    {
                        for(note=127;note>=0;--note)
                        {
                            if(MidiKeyState[note])
                            {
                                SynthChannelChangeNote(0,note);

                                break;
                            }
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

                case 0x01: MidiModulationDepth = (float)msg[2]/(128.0f*4.0f); break;

                case 0x06:
                    MidiDataMSB = msg[2];

                    if (MidiRPNLSB == 0 && MidiRPNMSB == 0)
                        MidiPitchBendRange = (float)MidiDataMSB * .5f;

                    break;

                default:
                    if (msg[1] >= 0x7b) { //all notes off and mono/poly mode changes that also requires to do all notes off
                        memset(MidiKeyState, 0, sizeof(MidiKeyState));

                        for(chn=0; chn < MaxSynthChannels; ++chn) SynthStopEnvelope(chn);
                    }
                    break;
                }
                break;

#if 0
            case 0xc0: //program change
            {
                if(Program!=msg[1])
                {
                    Program=msg[1];

                    UpdateGuiFlag=true;
                }
            }
            break;
#endif

            case 0xe0:  //pitch bend change
            {
                int32_t wheel = msg[1] | (msg[2] << 7);
                MidiPitchBend = (float)((wheel - 0x2000) * MidiPitchBendRange / 8192.0);
            }
            break;
            }
        }

        if(!poly&&key_off)
        {
            if(!MidiIsAnyKeyDown())
            {
                for(chn=0;chn<MaxSynthChannels;++chn) SynthStopEnvelope(chn);
            }
        }

        sEnvelopeDiv+=ENVELOPE_UPDATE_RATE_HZ/sampleRate;

        if(sEnvelopeDiv>=1.0f)
        {
            sEnvelopeDiv-=1.0f;

            SynthAdvanceEnvelopes();
        }

        for(chn=0;chn<MaxSynthChannels;++chn)
        {
            if(SynthChannel[chn].note<0) continue;

            //process lfo, per channel

            while(SynthChannel[chn].lfo_count>=pi) SynthChannel[chn].lfo_count-=pi;

            depth=1.0f*(1.0f-Program.values[pIdEnvLfoDepth])+SynthChannel[chn].ef_level*Program.values[pIdEnvLfoDepth];    //balance between just lfo and influenced by envelope

            SynthChannel[chn].lfo_out=sinf(SynthChannel[chn].lfo_count)*depth;

            SynthChannel[chn].lfo_count+=Program.values[pIdLfoSpeed]*LFO_MAX_HZ/sampleRate;

            //process slide

            if(SynthChannel[chn].slide_delay>0)
            {
                --SynthChannel[chn].slide_delay;
            }
            else
            {
                if(!slide||slide==1) SynthChannel[chn].slide_osca+=(Program.values[pIdSlideSpeed]*2.0f-1.0f)*.01f;
                if(!slide||slide==2) SynthChannel[chn].slide_oscb+=(Program.values[pIdSlideSpeed]*2.0f-1.0f)*.01f;
            }

            //process osc cut

            if(SynthChannel[chn].osca.cut>0) --SynthChannel[chn].osca.cut;
            if(SynthChannel[chn].oscb.cut>0) --SynthChannel[chn].oscb.cut;

            //process portamento

            if(Program.values[pIdPortaSpeed]>=1.0f)
            {
                SynthChannel[chn].freq=SynthChannel[chn].freq_new;
            }
            else
            {
                if(SynthChannel[chn].freq<SynthChannel[chn].freq_new)
                {
                    SynthChannel[chn].freq+=sSlideStep/sampleRate;

                    if(SynthChannel[chn].freq>SynthChannel[chn].freq_new) SynthChannel[chn].freq=SynthChannel[chn].freq_new;
                }

                if(SynthChannel[chn].freq>SynthChannel[chn].freq_new)
                {
                    SynthChannel[chn].freq+=sSlideStep/sampleRate;

                    if(SynthChannel[chn].freq<SynthChannel[chn].freq_new) SynthChannel[chn].freq=SynthChannel[chn].freq_new;
                }
            }

            //process vibrato

            vibrato=Program.values[pIdLfoPitchDepth]*SynthChannel[chn].lfo_out;

            //get adders out of frequencies

            detune=Program.values[pIdOscBDetune]+Program.values[pIdEnvOscBDetuneDepth]*.1f*SynthChannel[chn].ef_level;

            if(detune<0.0f) detune=0.0f;
            if(detune>1.0f) detune=1.0f;

            SynthChannel[chn].osca.add=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+vibrato+SynthChannel[chn].slide_osca+MidiPitchBend)/12.0f)*FloatToMultiple(Program.values[pIdOscAMultiple])/sampleRate/OVERSAMPLING;
            SynthChannel[chn].oscb.add=440.0f*pow(2.0f,(SynthChannel[chn].freq+modulation+vibrato+SynthChannel[chn].slide_oscb+MidiPitchBend+detune*DETUNE_SEMITONES)/12.0f)*FloatToMultiple(Program.values[pIdOscBMultiple])/sampleRate/OVERSAMPLING;

            //process filter

            if(Program.values[pIdFltCutoff]<1.0f)    //calculate filter coefficients
            {
                SynthChannel[chn].filter_resofreq=Program.values[pIdFltCutoff]+(SynthChannel[chn].velocity*(Program.values[pIdVelFltCutoff]*2.0f-1.0f))+(SynthChannel[chn].ef_level*(Program.values[pIdEnvFltDepth]*2.0f-1.0f))+(SynthChannel[chn].lfo_out*(Program.values[pIdLfoFltDepth]*2.0f-1.0f));
                SynthChannel[chn].filter_resofreq*=FILTER_CUTOFF_MAX_HZ;

                if(SynthChannel[chn].filter_resofreq<FILTER_CUTOFF_MIN_HZ) SynthChannel[chn].filter_resofreq=FILTER_CUTOFF_MIN_HZ;
                if(SynthChannel[chn].filter_resofreq>FILTER_CUTOFF_MAX_HZ) SynthChannel[chn].filter_resofreq=FILTER_CUTOFF_MAX_HZ;

                SynthChannel[chn].filter_amp=FILTER_MIN_RESONANCE+(Program.values[pIdFltReso]+SynthChannel[chn].velocity*(Program.values[pIdVelFltReso]*2.0f-1.0f))*FILTER_MAX_RESONANCE;

                if(SynthChannel[chn].filter_amp<FILTER_MIN_RESONANCE) SynthChannel[chn].filter_amp=FILTER_MIN_RESONANCE;
                if(SynthChannel[chn].filter_amp>FILTER_MAX_RESONANCE) SynthChannel[chn].filter_amp=FILTER_MAX_RESONANCE;

                w=2.0*M_PI*SynthChannel[chn].filter_resofreq/sampleRate;        // Pole angle
                q=1.0-w/(2.0*(SynthChannel[chn].filter_amp+0.5/(1.0+w))+w-2.0); // Pole magnitude

                SynthChannel[chn].filter_r=q*q;
                SynthChannel[chn].filter_c=SynthChannel[chn].filter_r+1.0-2.0*cos(w)*q;
            }
        }

        level_mix=0;

        for(chn=0;chn<MaxSynthChannels;++chn)
        {
            if(SynthChannel[chn].note<0) continue;

            dutya=Program.values[pIdOscADuty]+(Program.values[pIdVelOscADepth]*2.0f-1.0f)*SynthChannel[chn].velocity+(Program.values[pIdEnvOscADepth]*2.0f-1.0f)*SynthChannel[chn].ef_level+(Program.values[pIdLfoOscADepth]*2.0f-1.0f)*SynthChannel[chn].lfo_out;
            dutyb=Program.values[pIdOscBDuty]+(Program.values[pIdVelOscBDepth]*2.0f-1.0f)*SynthChannel[chn].velocity+(Program.values[pIdEnvOscBDepth]*2.0f-1.0f)*SynthChannel[chn].ef_level+(Program.values[pIdLfoOscBDepth]*2.0f-1.0f)*SynthChannel[chn].lfo_out;

            if(dutya<0.0f) dutya=0.0f;
            if(dutyb<0.0f) dutyb=0.0f;
            if(dutya>1.0f) dutya=1.0f;
            if(dutyb>1.0f) dutyb=1.0f;

            osc_balance=Program.values[pIdOscBalance]+(Program.values[pIdVelOscMixDepth]*2.0f-1.0f)*SynthChannel[chn].velocity+(Program.values[pIdEnvOscMixDepth]*2.0f-1.0f)*SynthChannel[chn].ef_level+(Program.values[pIdLfoOscMixDepth]*2.0f-1.0f)*SynthChannel[chn].lfo_out;

            if(osc_balance<0.0f) osc_balance=0.0f;
            if(osc_balance>1.0f) osc_balance=1.0f;

            level_osc=0;

            osc_mute=0;

            if(Program.values[pIdOscACut]>0) if(SynthChannel[chn].osca.cut<=0) osc_mute|=1;
            if(Program.values[pIdOscBCut]>0) if(SynthChannel[chn].oscb.cut<=0) osc_mute|=2;

            for(s=0;s<OVERSAMPLING;++s)
            {
                SynthChannel[chn].osca.acc+=SynthChannel[chn].osca.add;
                SynthChannel[chn].oscb.acc+=SynthChannel[chn].oscb.add;

                while(SynthChannel[chn].osca.acc>=1.0f) { SynthChannel[chn].osca.acc-=1.0f; ++SynthChannel[chn].osca.noise; }
                while(SynthChannel[chn].oscb.acc>=1.0f) { SynthChannel[chn].oscb.acc-=1.0f; ++SynthChannel[chn].oscb.noise; }

                if(!(osc_mute&1)) osca=SynthGetSample(&SynthChannel[chn].osca,overa,dutya,oscaw); else osca=0;
                if(!(osc_mute&2)) oscb=SynthGetSample(&SynthChannel[chn].oscb,overb,dutyb,oscbw); else oscb=0;

                switch(mix_mode)
                {
                case 0: level_osc+=osca*(1.0f-osc_balance)*.5f+oscb*osc_balance*.5f; break;
                case 1: level_osc+=(osca+(osc_balance*.5f))*(oscb-(osc_balance*.5f)); break;
                }
            }

            level_osc=level_osc*SynthChannel[chn].ev_level*SynthChannel[chn].volume/OVERSAMPLING/(MaxSynthChannels/2);

            //12db filter

            if(Program.values[pIdFltCutoff]<1.0f)
            {
                SynthChannel[chn].filter_vibraspeed+=(level_osc*32768.0-SynthChannel[chn].filter_vibrapos)*SynthChannel[chn].filter_c;

                SynthChannel[chn].filter_vibrapos+=SynthChannel[chn].filter_vibraspeed;

                SynthChannel[chn].filter_vibraspeed*=SynthChannel[chn].filter_r;

                level_osc=(float)SynthChannel[chn].filter_vibrapos;

                if(level_osc<-32768.0f) level_osc=-32768.0f; else if(level_osc>=32768.0f) level_osc=32768.0f;

                level_osc/=32768.0;
            }

            level_mix+=level_osc;
        }

        level_mix=level_mix*Program.values[pIdOutputGain];

        (*outL++)=level_mix;
        (*outR++)=level_mix;
    }

    // if(UpdateGuiFlag)
    // {
    //     UpdateGUI(true);
    //
    //     UpdateGuiFlag=false;
    // }
}

int32_t ChipWavePlugin::SynthAllocateVoice(int32_t note)
{
    int32_t chn;

    for (chn=0;chn<MaxSynthChannels;++chn) if (SynthChannel[chn].note==note) return chn;
    for (chn=0;chn<MaxSynthChannels;++chn) if (SynthChannel[chn].note<0) return chn;
    for (chn=0;chn<MaxSynthChannels;++chn) if (SynthChannel[chn].ev_stage==eStageRelease) return chn;

    return -1;
}

void ChipWavePlugin::SynthChannelChangeNote(int32_t chn, int32_t note)
{
    SynthChannel[chn].note=note;

    if (note>=0)
    {
        SynthChannel[chn].freq_new=(float)SynthChannel[chn].note-69;

        sSlideStep=(SynthChannel[chn].freq-SynthChannel[chn].freq_new)*20.0f*log(1.0f-Program.values[pIdPortaSpeed]);
    }
}

void ChipWavePlugin::SynthRestartEnvelope(int32_t chn)
{
    if (Program.values[pIdAmpAttack]>0||Program.values[pIdAmpDecay]>0)
    {
        SynthChannel[chn].ev_stage=eStageAttack;
        SynthChannel[chn].ev_level=0;
        SynthChannel[chn].ev_delta=SynthEnvelopeTimeToDelta(Program.values[pIdAmpAttack],ENVELOPE_ATTACK_MAX_MS);
    }
    else
    {
        SynthChannel[chn].ev_stage=eStageSustain;
        SynthChannel[chn].ev_level=Program.values[pIdAmpSustain];
    }

    if (Program.values[pIdEnvAttack]>0||Program.values[pIdEnvDecay]>0)
    {
        SynthChannel[chn].ef_stage=eStageAttack;
        SynthChannel[chn].ef_level=0;
        SynthChannel[chn].ef_delta=SynthEnvelopeTimeToDelta(Program.values[pIdEnvAttack],ENVELOPE_ATTACK_MAX_MS);
    }
    else
    {
        SynthChannel[chn].ef_stage=eStageSustain;
        SynthChannel[chn].ef_level=Program.values[pIdEnvSustain];
    }
}

void ChipWavePlugin::SynthStopEnvelope(int32_t chn)
{
    SynthChannel[chn].ev_stage=eStageRelease;
    SynthChannel[chn].ef_stage=eStageRelease;

    SynthChannel[chn].ev_delta=SynthEnvelopeTimeToDelta(Program.values[pIdAmpRelease],ENVELOPE_RELEASE_MAX_MS)*SynthChannel[chn].ev_level;
    SynthChannel[chn].ef_delta=SynthEnvelopeTimeToDelta(Program.values[pIdEnvRelease],ENVELOPE_RELEASE_MAX_MS)*SynthChannel[chn].ef_level;

    if (Program.values[pIdSlideDelay]>=1.0f) SynthChannel[chn].slide_delay=0;
}

void ChipWavePlugin::SynthAdvanceEnvelopes()
{
    int32_t chn;

    for (chn=0;chn<MaxSynthChannels;++chn)
    {
        //volume

        if (SynthChannel[chn].ev_stage==eStageReset)
        {
            SynthChannel[chn].note=-1;
            SynthChannel[chn].ev_level=0;
        }
        else
        {
            switch(SynthChannel[chn].ev_stage)
            {
            case eStageAttack:
                {
                    SynthChannel[chn].ev_level+=SynthChannel[chn].ev_delta;

                    if (SynthChannel[chn].ev_level>=1.0f)
                    {
                        SynthChannel[chn].ev_level=1.0f;
                        SynthChannel[chn].ev_delta=SynthEnvelopeTimeToDelta(Program.values[pIdAmpDecay],ENVELOPE_DECAY_MAX_MS);
                        SynthChannel[chn].ev_stage=eStageDecay;
                    }
                }
                break;

            case eStageDecay:
                {
                    SynthChannel[chn].ev_level-=SynthChannel[chn].ev_delta;

                    if (SynthChannel[chn].ev_level<=Program.values[pIdAmpSustain])
                    {
                        SynthChannel[chn].ev_level=Program.values[pIdAmpSustain];
                        SynthChannel[chn].ev_stage=eStageSustain;
                    }
                }
                break;

            case eStageRelease:
                {
                    SynthChannel[chn].ev_level-=SynthChannel[chn].ev_delta;

                    if (SynthChannel[chn].ev_level<=0)
                    {
                        SynthChannel[chn].ev_level=0;
                        SynthChannel[chn].ev_stage=eStageReset;
                    }
                }
                break;
            }
        }

        //filter

        if (SynthChannel[chn].ef_stage==eStageReset)
        {
            SynthChannel[chn].ef_level=0;
        }
        else
        {
            switch(SynthChannel[chn].ef_stage)
            {
            case eStageAttack:
                {
                    SynthChannel[chn].ef_level+=SynthChannel[chn].ef_delta;

                    if (SynthChannel[chn].ef_level>=1.0f)
                    {
                        SynthChannel[chn].ef_level=1.0f;
                        SynthChannel[chn].ef_delta=SynthEnvelopeTimeToDelta(Program.values[pIdEnvDecay],ENVELOPE_DECAY_MAX_MS);
                        SynthChannel[chn].ef_stage=eStageDecay;
                    }
                }
                break;

            case eStageDecay:
                {
                    SynthChannel[chn].ef_level-=SynthChannel[chn].ef_delta;

                    if (SynthChannel[chn].ef_level<=Program.values[pIdEnvSustain])
                    {
                        SynthChannel[chn].ef_level=Program.values[pIdEnvSustain];
                        SynthChannel[chn].ef_stage=eStageSustain;
                    }
                }
                break;

            case eStageRelease:
                {
                    SynthChannel[chn].ef_level-=SynthChannel[chn].ef_delta;

                    if (SynthChannel[chn].ef_level<=0)
                    {
                        SynthChannel[chn].ef_level=0;
                        SynthChannel[chn].ef_stage=eStageReset;
                    }
                }
                break;
            }
        }
    }
}

bool ChipWavePlugin::MidiIsAnyKeyDown()
{
    int i;

    for (i=0;i<128;++i) if (MidiKeyState[i]) return true;

    return false;
}

///
namespace DISTRHO {

Plugin *createPlugin()
{
    return new ChipWavePlugin;
}

} // namespace DISTRHO
