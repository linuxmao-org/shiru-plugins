#include "ChipDrumPlugin.hpp"
#include <cstring>

ChipDrumPlugin::ChipDrumPlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count)
{
    loadProgram(0);

    memset(SynthChannel, 0, sizeof(SynthChannel));

    RandomNumberGenerator.seed(1);
}

const char *ChipDrumPlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *ChipDrumPlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *ChipDrumPlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *ChipDrumPlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *ChipDrumPlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t ChipDrumPlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t ChipDrumPlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void ChipDrumPlugin::initParameter(uint32_t index, Parameter &parameter)
{
    InitParameter(index, parameter);
}

float ChipDrumPlugin::getParameterValue(uint32_t index) const
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count, 0);

    return Program.values[index];
}

void ChipDrumPlugin::setParameterValue(uint32_t index, float value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < Parameter_Count,);

    Program.values[index] = value;
}

void ChipDrumPlugin::initProgramName(uint32_t index, String &name)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    name = PresetData[index].name;
}

void ChipDrumPlugin::loadProgram(uint32_t index)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < PresetData.size(), );

    setState("ProgramName", PresetData[index].name);

    for (unsigned p = 0; p < Parameter_Count; ++p)
        Program.values[p] = PresetData[index].values[p];
}

void ChipDrumPlugin::initState(uint32_t index, String &state_key, String &default_value)
{
    DISTRHO_SAFE_ASSERT_RETURN(index < State_Count, );

    switch (index) {
    case 0:
        state_key = "ProgramName";
        default_value = "default";
        break;
    }
}

String ChipDrumPlugin::getState(const char *key) const
{
    if (!strcmp(key, "ProgramName"))
        return String(Program.name);

    DISTRHO_SAFE_ASSERT_RETURN(false, String());
}

void ChipDrumPlugin::setState(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramName"))
        memcpy(Program.name, value, strnlen(value, MAX_NAME_LEN) + 1);
}

void ChipDrumPlugin::run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count)
{
    float *outL[DISTRHO_PLUGIN_NUM_OUTPUTS/2],*outR[DISTRHO_PLUGIN_NUM_OUTPUTS/2];
    float level_l[DISTRHO_PLUGIN_NUM_OUTPUTS/2],level_r[DISTRHO_PLUGIN_NUM_OUTPUTS/2];
    float vol,out,tone_output,noise_output,pan;
    float normal_output,filter_output;
    int32_t s,nv,chn,note,notem;
    double q,w;
    bool update;

    outL[0]=outputs[0];
    outR[0]=outputs[1];
    outL[1]=outputs[2];
    outR[1]=outputs[3];
    outL[2]=outputs[4];
    outR[2]=outputs[5];
    outL[3]=outputs[6];
    outR[3]=outputs[7];

    double sampleRate=getSampleRate();

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
                notem=note;

                switch(note)
                {
                case 0:  chn=0; break;//C
                case 1:  chn=8; break;//C#
                case 2:  chn=1; break;//D
                case 3:  chn=7; break;//D#
                case 4:  chn=2; break;//E
                case 5:  chn=3; break;//F
                case 6:  chn=7; note=6; break;//F#
                case 7:  chn=4; note=7; break;//G
                case 8:  chn=7; note=6; break;//G#
                case 9:  chn=5; note=7; break;//A
                case 10: chn=7; note=6; break;//A#
                case 11: chn=6; note=7; break;//B
                }

                if(msg[0] == 0x90 && msg[2] > 0)//key on
                {
                    SynthChannel[chn].note=note;
                    SynthChannel[chn].notem=notem;

                    SynthChannel[chn].velocity=((float)msg[2]/100.0f);

                    SynthRestartTone(chn);
                    SynthRestartNoise(chn);

                    vol=Program.values[pIdDrumVolume1 + note]*(1.0f*(1.0f-Program.values[pIdVelDrumVolume1 + note])+SynthChannel[chn].velocity*Program.values[pIdVelDrumVolume1 + note]);

                    pan=Program.values[pIdDrumPan1 + note];

                    if(notem==6)  pan-=Program.values[pIdHatPanWidth]*.5f;
                    if(notem==7)  pan-=Program.values[pIdTomPanWidth]*.5f;
                    if(notem==10) pan+=Program.values[pIdHatPanWidth]*.5f;
                    if(notem==11) pan+=Program.values[pIdTomPanWidth]*.5f;

                    if(pan<0.0f) pan=0.0f;
                    if(pan>1.0f) pan=1.0f;

                    SynthChannel[chn].volume_l=vol*(1.0f-pan);
                    SynthChannel[chn].volume_r=vol*pan;

                    SynthChannel[chn].tone_wave=(int32_t)(Program.values[pIdToneWave1 + note]*3.99f);

                    SynthChannel[chn].tone_over=OverdriveValue(Program.values[pIdToneOver1 + note]+(Program.values[pIdVelToneOver1 + note]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.5f);

                    SynthChannel[chn].group=(int32_t)(Program.values[pIdDrumGroup1 + note]*((DISTRHO_PLUGIN_NUM_OUTPUTS/2)-.1f));

                    SynthChannel[chn].bit_depth=pBitDepth[(int32_t)(Program.values[pIdDrumBitDepth1 + note]*7.99f)];

                    SynthChannel[chn].frame_acc=1.0f;    //force first update

                    if(Program.values[pIdDrumUpdateRate1 + note]<1.0f)
                    {
                        SynthChannel[chn].frame_add=1.0f/(sampleRate/(MIN_UPDATE_RATE+Program.values[pIdDrumUpdateRate1 + note]*MAX_UPDATE_RATE));
                    }
                    else
                    {
                        SynthChannel[chn].frame_add=0;    //0 for instant update
                    }

                    //calculate filter coefficients, does not change during note play

                    SynthChannel[chn].filter_route=(int32_t)(Program.values[pIdFilterRoute1 + note]*3.99f);

                    if(Program.values[pIdFilterLP1 + note]<1.0f)
                    {
                        SynthChannel[chn].lpf_resofreq=Program.values[pIdFilterLP1 + note]*FILTER_CUTOFF_MAX_HZ;
                    }
                    else
                    {
                        SynthChannel[chn].lpf_resofreq=0;
                    }

                    //if(SynthChannel[chn].lpf_resofreq<FILTER_CUTOFF_MIN_HZ) SynthChannel[chn].lpf_resofreq=FILTER_CUTOFF_MIN_HZ;
                    //if(SynthChannel[chn].lpf_resofreq>FILTER_CUTOFF_MAX_HZ) SynthChannel[chn].lpf_resofreq=FILTER_CUTOFF_MAX_HZ;

                    w=2.0*M_PI*SynthChannel[chn].lpf_resofreq/sampleRate;        // Pole angle
                    q=1.0-w/(2.0*(1.0+0.5/(1.0+w))+w-2.0); // Pole magnitude

                    SynthChannel[chn].lpf_r=q*q;
                    SynthChannel[chn].lpf_c=SynthChannel[chn].lpf_r+1.0-2.0*cos(w)*q;

                    if(Program.values[pIdFilterHP1 + note]>0.0f)
                    {
                        SynthChannel[chn].hpf_resofreq=Program.values[pIdFilterHP1 + note]*FILTER_CUTOFF_MAX_HZ;
                    }
                    else
                    {
                        SynthChannel[chn].hpf_resofreq=0;
                    }

                    //if(SynthChannel[chn].hpf_resofreq<FILTER_CUTOFF_MIN_HZ) SynthChannel[chn].hpf_resofreq=FILTER_CUTOFF_MIN_HZ;
                    //if(SynthChannel[chn].hpf_resofreq>FILTER_CUTOFF_MAX_HZ) SynthChannel[chn].hpf_resofreq=FILTER_CUTOFF_MAX_HZ;

                    w=2.0*M_PI*SynthChannel[chn].hpf_resofreq/sampleRate;        // Pole angle
                    q=1.0-w/(2.0*(1.0+0.5/(1.0+w))+w-2.0); // Pole magnitude

                    SynthChannel[chn].hpf_r=q*q;
                    SynthChannel[chn].hpf_c=SynthChannel[chn].hpf_r+1.0-2.0*cos(w)*q;

                    SynthChannel[chn].lpf_vibrapos=0;
                    SynthChannel[chn].lpf_vibraspeed=0;
                    SynthChannel[chn].hpf_vibrapos=0;
                    SynthChannel[chn].hpf_vibraspeed=0;

                    //retrigger

                    SynthChannel[chn].retrigger_acc=0;
                    SynthChannel[chn].retrigger_add=1.0f/(RETRIGGER_MAX_MS*Program.values[pIdRetrigTime1 + note]/1000.0f*sampleRate);
                    SynthChannel[chn].retrigger_count=(int32_t)(Program.values[pIdRetrigCount1 + note]*RETRIGGER_MAX_COUNT);
                    SynthChannel[chn].retrigger_route=(int32_t)(Program.values[pIdRetrigRoute1 + note]*2.99f);
                }
            }
            break;

#if 0
            case 0xc0:
            {
                Program=msg[1];

                UpdateGuiFlag=true;
            }
            break;
#endif
            }
        }

        level_l[0]=0;
        level_l[1]=0;
        level_l[2]=0;
        level_l[3]=0;
        level_r[0]=0;
        level_r[1]=0;
        level_r[2]=0;
        level_r[3]=0;

        for(chn=0;chn<(int32_t)SYNTH_CHANNELS;++chn)
        {
            //frame counter

            update=false;

            if(SynthChannel[chn].frame_add>0)
            {
                SynthChannel[chn].frame_acc+=SynthChannel[chn].frame_add;

                while(SynthChannel[chn].frame_acc>=1.0f)
                {
                    SynthChannel[chn].frame_acc-=1.0f;

                    update=true;
                }
            }
            else
            {
                update=true;
            }

            if(update)
            {
                SynthChannel[chn].tone_adda  =SynthChannel[chn].tone_add;
                SynthChannel[chn].noise_add1a=SynthChannel[chn].noise_add1;
                SynthChannel[chn].noise_add2a=SynthChannel[chn].noise_add2;
            }

            tone_output=0;
            noise_output=0;

            for(s=0;s<(int32_t)OVERSAMPLING;++s)
            {
                //process tone

                if(SynthChannel[chn].tone_level>0)
                {
                    //envelope

                    if(SynthChannel[chn].tone_env_acc<1.0f)
                    {
                        SynthChannel[chn].tone_env_acc+=SynthChannel[chn].tone_env_add1;

                        SynthChannel[chn].tone_level-=SynthChannel[chn].tone_decay;

                        if(SynthChannel[chn].tone_level<SynthChannel[chn].tone_sustain) SynthChannel[chn].tone_level=SynthChannel[chn].tone_sustain;
                    }
                    else
                    {
                        SynthChannel[chn].tone_env_acc+=SynthChannel[chn].tone_env_add2;

                        SynthChannel[chn].tone_level-=SynthChannel[chn].tone_release;

                        if(SynthChannel[chn].tone_level<0) SynthChannel[chn].tone_level=0;
                    }

                    //oscillator

                    SynthChannel[chn].tone_acc+=SynthChannel[chn].tone_adda;
                    SynthChannel[chn].tone_add+=SynthChannel[chn].tone_delta;

                    while(SynthChannel[chn].tone_acc>=1.0f) SynthChannel[chn].tone_acc-=1.0f;

                    if(SynthChannel[chn].tone_add<0.0f) SynthChannel[chn].tone_add=0.0f;
                    if(SynthChannel[chn].tone_add>1.0f) SynthChannel[chn].tone_add=1.0f;

                    SynthChannel[chn].tone_sample=SynthGetSample(SynthChannel[chn].tone_wave,SynthChannel[chn].tone_acc,SynthChannel[chn].tone_over)*SynthChannel[chn].tone_level;

                    tone_output+=SynthChannel[chn].tone_sample;
                }

                //process noise

                if(SynthChannel[chn].noise_level>0)
                {
                    //envelope

                    if(SynthChannel[chn].noise_env_acc<1.0f)
                    {
                        SynthChannel[chn].noise_env_acc+=SynthChannel[chn].noise_env_add1;

                        SynthChannel[chn].noise_level-=SynthChannel[chn].noise_decay;

                        if(SynthChannel[chn].noise_level<SynthChannel[chn].noise_sustain) SynthChannel[chn].noise_level=SynthChannel[chn].noise_sustain;
                    }
                    else
                    {
                        SynthChannel[chn].noise_env_acc+=SynthChannel[chn].noise_env_add2;

                        SynthChannel[chn].noise_level-=SynthChannel[chn].noise_release;

                        if(SynthChannel[chn].noise_level<0) SynthChannel[chn].noise_level=0;
                    }

                    //burst sequencer and oscillator

                    if(SynthChannel[chn].noise_frame_acc>=1.0f&&SynthChannel[chn].noise_frame_acc<2.0f)
                    {
                        SynthChannel[chn].noise_acc+=SynthChannel[chn].noise_add2a;
                    }
                    else
                    {
                        SynthChannel[chn].noise_acc+=SynthChannel[chn].noise_add1a;
                    }

                    while(SynthChannel[chn].noise_acc>=1.0f)
                    {
                        SynthChannel[chn].noise_acc-=1.0f;

                        ++SynthChannel[chn].noise_ptr;
                    }

                    nv=Noise[((SynthChannel[chn].noise_ptr&SynthChannel[chn].noise_mask)+SynthChannel[chn].noise_seed)&65535];

                    if(SynthChannel[chn].noise_type)
                    {
                        SynthChannel[chn].noise_sample=(nv&1)?SynthChannel[chn].noise_level*.75f:0;
                    }
                    else
                    {
                        SynthChannel[chn].noise_sample=(float)nv*(1.0f/256.0f)*SynthChannel[chn].noise_level;
                    }

                    noise_output+=SynthChannel[chn].noise_sample;
                }

                if(SynthChannel[chn].noise_frame_acc<1.0f)
                {
                    SynthChannel[chn].noise_frame_acc+=SynthChannel[chn].noise_frame_add1;
                }
                else
                {
                    SynthChannel[chn].noise_frame_acc+=SynthChannel[chn].noise_frame_add2;
                }
            }

            tone_output/=OVERSAMPLING;
            noise_output/=OVERSAMPLING;

            //apply bit reduction if needed

            if(SynthChannel[chn].bit_depth>0)
            {
                tone_output =floorf(tone_output *SynthChannel[chn].bit_depth)/SynthChannel[chn].bit_depth;
                noise_output=floorf(noise_output*SynthChannel[chn].bit_depth)/SynthChannel[chn].bit_depth;
            }

            //filter

            switch(SynthChannel[chn].filter_route)
            {
            case 0:
                normal_output=0;
                filter_output=tone_output+noise_output;
                break;

            case 1:
                normal_output=noise_output;
                filter_output=tone_output;
                break;

            case 2:
                normal_output=tone_output;
                filter_output=noise_output;
                break;

            default:
                normal_output=tone_output+noise_output;
                filter_output=0;
            }

            //12db lpf filter

            if(SynthChannel[chn].lpf_resofreq>0)
            {
                SynthChannel[chn].lpf_vibraspeed+=(filter_output*32768.0-SynthChannel[chn].lpf_vibrapos)*SynthChannel[chn].lpf_c;

                SynthChannel[chn].lpf_vibrapos+=SynthChannel[chn].lpf_vibraspeed;

                SynthChannel[chn].lpf_vibraspeed*=SynthChannel[chn].lpf_r;

                filter_output=(float)SynthChannel[chn].lpf_vibrapos;

                filter_output/=32768.0;
            }

            //12db hpf filter

            if(SynthChannel[chn].hpf_resofreq>0)
            {
                SynthChannel[chn].hpf_vibraspeed+=(filter_output*32768.0-SynthChannel[chn].hpf_vibrapos)*SynthChannel[chn].hpf_c;

                SynthChannel[chn].hpf_vibrapos+=SynthChannel[chn].hpf_vibraspeed;

                SynthChannel[chn].hpf_vibraspeed*=SynthChannel[chn].hpf_r;

                out=(float)SynthChannel[chn].hpf_vibrapos;

                out/=32768.0;

                filter_output-=out;
            }

            //mix outputs

            out=normal_output+filter_output;

            level_l[SynthChannel[chn].group]+=out*SynthChannel[chn].volume_l;
            level_r[SynthChannel[chn].group]+=out*SynthChannel[chn].volume_r;

            //retrigger

            if(SynthChannel[chn].retrigger_count)
            {
                SynthChannel[chn].retrigger_acc+=SynthChannel[chn].retrigger_add;

                if(SynthChannel[chn].retrigger_acc>=1.0f)
                {
                    SynthChannel[chn].retrigger_acc-=1.0f;

                    if(SynthChannel[chn].retrigger_route<2) SynthRestartTone(chn);
                    if(SynthChannel[chn].retrigger_route!=1) SynthRestartNoise(chn);

                    --SynthChannel[chn].retrigger_count;
                }
            }
        }

        level_l[0]*=Program.values[pIdOutputGain];
        level_l[1]*=Program.values[pIdOutputGain];
        level_l[2]*=Program.values[pIdOutputGain];
        level_l[3]*=Program.values[pIdOutputGain];
        level_r[0]*=Program.values[pIdOutputGain];
        level_r[1]*=Program.values[pIdOutputGain];
        level_r[2]*=Program.values[pIdOutputGain];
        level_r[3]*=Program.values[pIdOutputGain];

        if(level_l[0]<-1.0f) level_l[0]=-1.0f; else if(level_l[0]> 1.0f) level_l[0]= 1.0f;
        if(level_l[1]<-1.0f) level_l[1]=-1.0f; else if(level_l[1]> 1.0f) level_l[1]= 1.0f;
        if(level_l[2]<-1.0f) level_l[2]=-1.0f; else if(level_l[2]> 1.0f) level_l[2]= 1.0f;
        if(level_l[3]<-1.0f) level_l[3]=-1.0f; else if(level_l[3]> 1.0f) level_l[3]= 1.0f;
        if(level_r[0]<-1.0f) level_r[0]=-1.0f; else if(level_r[0]> 1.0f) level_r[0]= 1.0f;
        if(level_r[1]<-1.0f) level_r[1]=-1.0f; else if(level_r[1]> 1.0f) level_r[1]= 1.0f;
        if(level_r[2]<-1.0f) level_r[2]=-1.0f; else if(level_r[2]> 1.0f) level_r[2]= 1.0f;
        if(level_r[3]<-1.0f) level_r[3]=-1.0f; else if(level_r[3]> 1.0f) level_r[3]= 1.0f;

        (*outL[0]++)=level_l[0];
        (*outR[0]++)=level_r[0];
        (*outL[1]++)=level_l[1];
        (*outR[1]++)=level_r[1];
        (*outL[2]++)=level_l[2];
        (*outR[2]++)=level_r[2];
        (*outL[3]++)=level_l[3];
        (*outR[3]++)=level_r[3];
    }

    // if(UpdateGuiFlag)
    // {
    //     UpdateGUI(true);
    //
    //     UpdateGuiFlag=false;
    // }
}

void ChipDrumPlugin::SynthRestartTone(int32_t chn)
{
    float dc,rl,freq,div1,div2;

    int note = SynthChannel[chn].note;

    float sampleRate = getSampleRate();

    dc=Program.values[pIdToneDecay1 + note];
    rl=Program.values[pIdToneRelease1 + note];

    if(SynthChannel[chn].notem==6)
    {
        dc+=(Program.values[pIdHat1Length]-.5f);
        rl+=(Program.values[pIdHat1Length]-.5f);
    }

    if(SynthChannel[chn].notem==8)
    {
        dc+=(Program.values[pIdHat2Length]-.5f);
        rl+=(Program.values[pIdHat2Length]-.5f);
    }

    if(SynthChannel[chn].notem==10)
    {
        dc+=(Program.values[pIdHat3Length]-.5f);
        rl+=(Program.values[pIdHat3Length]-.5f);
    }

    if(dc<0) dc=0;
    if(dc>1) dc=1;
    if(rl<0) rl=0;
    if(rl>1) rl=1;

    div1=dc*(DECAY_TIME_MAX_MS  /1000.0f)*sampleRate;
    div2=rl*(RELEASE_TIME_MAX_MS/1000.0f)*sampleRate;

    SynthChannel[chn].tone_level  =Program.values[pIdToneLevel1 + note];
    SynthChannel[chn].tone_sustain=Program.values[pIdToneLevel1 + note]*Program.values[pIdToneSustain1 + note];
    SynthChannel[chn].tone_decay  =div1>0?((SynthChannel[chn].tone_level-SynthChannel[chn].tone_sustain)/div1/OVERSAMPLING):1.0f;
    SynthChannel[chn].tone_release=div2>0?(SynthChannel[chn].tone_sustain/div2/OVERSAMPLING):1.0f;

    SynthChannel[chn].tone_env_acc=0;
    SynthChannel[chn].tone_env_add1=div1>0?(1.0f/div1/OVERSAMPLING):1.0f;
    SynthChannel[chn].tone_env_add2=div2>0?(1.0f/div2/OVERSAMPLING):1.0f;

    if(SynthChannel[chn].tone_decay==0) SynthChannel[chn].tone_decay=.000001f;

    freq=Program.values[pIdTonePitch1 + note]+(Program.values[pIdVelTonePitch1 + note]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.1f;

    if(SynthChannel[chn].notem==7 ) freq+=(Program.values[pIdTom1Pitch]*2.0f-1.0f)*.1f;
    if(SynthChannel[chn].notem==9 ) freq+=(Program.values[pIdTom2Pitch]*2.0f-1.0f)*.1f;
    if(SynthChannel[chn].notem==11) freq+=(Program.values[pIdTom3Pitch]*2.0f-1.0f)*.1f;

    SynthChannel[chn].tone_acc=0;
    SynthChannel[chn].tone_add=FloatToHz(freq,TONE_PITCH_MAX_HZ)/sampleRate/OVERSAMPLING;
    SynthChannel[chn].tone_delta=(-1.0f+Program.values[pIdToneSlide1 + note]*2.0f)*(48000.0f/sampleRate)/(50.0f*sampleRate)/OVERSAMPLING;
}

void ChipDrumPlugin::SynthRestartNoise(int32_t chn)
{
    std::minstd_rand &rng = RandomNumberGenerator;

    float dc,rl,div1,div2,freq1,freq2;

    int note = SynthChannel[chn].note;

    float sampleRate = getSampleRate();

    dc=Program.values[pIdNoiseDecay1 + note];
    rl=Program.values[pIdNoiseRelease1 + note];

    if(SynthChannel[chn].notem==6)
    {
        dc+=(Program.values[pIdHat1Length]-.5f);
        rl+=(Program.values[pIdHat1Length]-.5f);
    }

    if(SynthChannel[chn].notem==8)
    {
        dc+=(Program.values[pIdHat2Length]-.5f);
        rl+=(Program.values[pIdHat2Length]-.5f);
    }

    if(SynthChannel[chn].notem==10)
    {
        dc+=(Program.values[pIdHat3Length]-.5f);
        rl+=(Program.values[pIdHat3Length]-.5f);
    }

    if(dc<0) dc=0;
    if(dc>1) dc=1;
    if(rl<0) rl=0;
    if(rl>1) rl=1;

    div1=dc*(DECAY_TIME_MAX_MS  /1000.0f)*sampleRate;
    div2=rl*(RELEASE_TIME_MAX_MS/1000.0f)*sampleRate;

    SynthChannel[chn].noise_level  =Program.values[pIdNoiseLevel1 + note];
    SynthChannel[chn].noise_sustain=Program.values[pIdNoiseLevel1 + note]*Program.values[pIdNoiseSustain1 + note];
    SynthChannel[chn].noise_decay  =div1>0?((SynthChannel[chn].noise_level-SynthChannel[chn].noise_sustain)/div1/OVERSAMPLING):1.0f;
    SynthChannel[chn].noise_release=div2>0?(SynthChannel[chn].noise_sustain/div2/OVERSAMPLING):1.0f;

    SynthChannel[chn].noise_env_acc=0;
    SynthChannel[chn].noise_env_add1=div1>0?(1.0f/div1/OVERSAMPLING):1.0f;
    SynthChannel[chn].noise_env_add2=div2>0?(1.0f/div2/OVERSAMPLING):1.0f;

    freq1=Program.values[pIdNoisePitch11 + note]+(Program.values[pIdVelNoisePitch1 + note]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.1f;
    freq2=Program.values[pIdNoisePitch21 + note]+(Program.values[pIdVelNoisePitch1 + note]*2.0f-1.0f)*(1.0f-SynthChannel[chn].velocity)*.1f;

    SynthChannel[chn].noise_acc=0;
    SynthChannel[chn].noise_add1=freq1*NOISE_PITCH_MAX_HZ/sampleRate/OVERSAMPLING;
    SynthChannel[chn].noise_add2=freq2*NOISE_PITCH_MAX_HZ/sampleRate/OVERSAMPLING;

    if(Program.values[pIdNoiseSeed1 + note]>0) SynthChannel[chn].noise_seed=(int32_t)(Program.values[pIdNoiseSeed1 + note]*65535.9f); else SynthChannel[chn].noise_seed=std::uniform_int_distribution<>{0, 32767}(rng);

    SynthChannel[chn].noise_type=Program.values[pIdNoiseType1 + note]<.5f?0:1;

    div1=Program.values[pIdNoisePitch2Off1 + note]*(DECAY_TIME_MAX_MS/1000.0f)*sampleRate;
    div2=Program.values[pIdNoisePitch2Len1 + note]*(NOISE_BURST_MAX_MS/1000.0f)*sampleRate;

    SynthChannel[chn].noise_ptr=0;
    SynthChannel[chn].noise_frame_acc=0;
    SynthChannel[chn].noise_frame_add1=div1>0?(1.0f/div1/OVERSAMPLING):1.0f;
    SynthChannel[chn].noise_frame_add2=div2>0?(1.0f/div2/OVERSAMPLING):1.0f;

    SynthChannel[chn].noise_mask=FloatToNoisePeriod(Program.values[pIdNoisePeriod1 + note]);
}

///
namespace DISTRHO {

Plugin *createPlugin()
{
    return new ChipDrumPlugin;
}

} // namespace DISTRHO
