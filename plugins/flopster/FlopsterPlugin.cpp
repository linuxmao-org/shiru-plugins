#include "FlopsterPlugin.hpp"

FlopsterPlugin::FlopsterPlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count)
{
    memset(&FDD, 0, sizeof(FDD));

    ///
    const sampleObject *samplePtr = SampleSet;
    for(unsigned i=0;i<STEP_SAMPLES_ALL;++i)
        SampleHeadStep[i] = *samplePtr++;
    for(unsigned i=0;i<HEAD_BUZZ_RANGE;++i)
        SampleHeadBuzz[i] = *samplePtr++;
    for(unsigned i=0;i<HEAD_SEEK_RANGE;++i)
        SampleHeadSeek[i] = *samplePtr++;
    SampleDiskPush = *samplePtr++;
    SampleDiskInsert = *samplePtr++;
    SampleDiskEject = *samplePtr++;
    SampleDiskPull = *samplePtr++;
    FDD.spindle_sample = *samplePtr++;

    ///
    memset(MidiKeyState,0,sizeof(MidiKeyState));

    ///
    for (unsigned p = 0; p < Parameter_Count; ++p) {
        Parameter param;
        initParameter(p, param);
        setParameterValue(p, param.ranges.def);
    }
}

const char *FlopsterPlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *FlopsterPlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *FlopsterPlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *FlopsterPlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *FlopsterPlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t FlopsterPlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t FlopsterPlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void FlopsterPlugin::initParameter(uint32_t index, Parameter &parameter)
{
    parameter.hints = kParameterIsAutomable;

    switch (index) {
    case pIdHeadStepGain:
        parameter.symbol = "HeadStepGain";
        parameter.name = "Head step gain";
        parameter.hints |= kParameterIsLogarithmic;
        parameter.ranges = ParameterRanges(1.0, 0.0, 1.0);
        break;
    case pIdHeadSeekGain:
        parameter.symbol = "HeadSeekGain";
        parameter.name = "Head seek gain";
        parameter.hints |= kParameterIsLogarithmic;
        parameter.ranges = ParameterRanges(1.0, 0.0, 1.0);
        break;
    case pIdHeadBuzzGain:
        parameter.symbol = "HeadBuzzGain";
        parameter.name = "Head buzz gain";
        parameter.hints |= kParameterIsLogarithmic;
        parameter.ranges = ParameterRanges(1.0, 0.0, 1.0);
        break;
    case pIdSpindleGain:
        parameter.symbol = "SpindleGain";
        parameter.name = "Spindle gain";
        parameter.hints |= kParameterIsLogarithmic;
        parameter.ranges = ParameterRanges(0.25, 0.0, 1.0);
        break;
    case pIdNoisesGain:
        parameter.symbol = "NoisesGain";
        parameter.name = "Noises gain";
        parameter.hints |= kParameterIsLogarithmic;
        parameter.ranges = ParameterRanges(0.5, 0.0, 1.0);
        break;
    case pIdOutputGain:
        parameter.symbol = "OutputGain";
        parameter.name = "Output gain";
        parameter.hints |= kParameterIsLogarithmic;
        parameter.ranges = ParameterRanges(1.0, 0.0, 1.0);
        break;
    default:
        DISTRHO_SAFE_ASSERT(false);
    }
}

float FlopsterPlugin::getParameterValue(uint32_t index) const
{
    switch (index) {
    case pIdHeadStepGain:
        return pHeadStepGain;
    case pIdHeadSeekGain:
        return pHeadSeekGain;
    case pIdHeadBuzzGain:
        return pHeadBuzzGain;
    case pIdSpindleGain:
        return pSpindleGain;
    case pIdNoisesGain:
        return pNoisesGain;
    case pIdOutputGain:
        return pOutputGain;
    default:
        DISTRHO_SAFE_ASSERT_RETURN(false, 0);
    }
}

void FlopsterPlugin::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case pIdHeadStepGain:
        pHeadStepGain = value; break;
    case pIdHeadSeekGain:
        pHeadSeekGain = value; break;
    case pIdHeadBuzzGain:
        pHeadBuzzGain = value; break;
    case pIdSpindleGain:
        pSpindleGain = value; break;
    case pIdNoisesGain:
        pNoisesGain = value; break;
    case pIdOutputGain:
        pOutputGain = value; break;
    default:
        DISTRHO_SAFE_ASSERT(false);
    }
}

void FlopsterPlugin::initState(uint32_t index, String &state_key, String &default_value)
{
    (void)state_key;
    (void)default_value;

    switch (index) {
    default:
        DISTRHO_SAFE_ASSERT(false);
    }
}

String FlopsterPlugin::getState(const char *key) const
{
    (void)key;

    DISTRHO_SAFE_ASSERT_RETURN(false, String());
}

void FlopsterPlugin::setState(const char *key, const char *value)
{
    (void)key;
    (void)value;

    DISTRHO_SAFE_ASSERT(false);
}

void FlopsterPlugin::run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count)
{
    float *outL=outputs[0];
    float *outR=outputs[1];
    float level;
    int note,type;
    double sample_step;
    bool prev_any,spindle_stop,head_stop,reset_low_freq=false;

    double sampleRate = getSampleRate();
    sample_step=44100.0/sampleRate;

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
            case 0x80:
            case 0x90:
            {
                prev_any=MidiIsAnyKeyDown();

                if(msg[2])//key on
                {
                    MidiKeyState[msg[1]]=msg[2];
                }
                else//key off
                {
                    MidiKeyState[msg[1]]=0;
                }

                spindle_stop=true;
                head_stop=true;

                for(note=127;note>=0;--note)
                {
                    if(MidiKeyState[note])
                    {
                        reset_low_freq=true;

                        if(note>=SPECIAL_NOTE)
                        {
                            switch(note)
                            {
                            case SPINDLE_NOTE:
                            {
                                spindle_stop=false;
                                reset_low_freq=false;
                            }
                            break;

                            case SINGLE_STEP_NOTE:
                            {
                                FloppyStep(-1);

                                MidiKeyState[note]=0;
                            }
                            break;

                            case DISK_PUSH_NOTE:
                            {
                                FloppyStartHeadSample(&SampleDiskPush,pNoisesGain,false,0);

                                MidiKeyState[note]=0;
                            }
                            break;

                            case DISK_INSERT_NOTE:
                            {
                                FloppyStartHeadSample(&SampleDiskInsert,pNoisesGain,false,0);

                                MidiKeyState[note]=0;
                            }
                            break;

                            case DISK_EJECT_NOTE:
                            {
                                FloppyStartHeadSample(&SampleDiskEject,pNoisesGain,false,0);

                                MidiKeyState[note]=0;
                            }
                            break;

                            case DISK_PULL_NOTE:
                            {
                                FloppyStartHeadSample(&SampleDiskPull,pNoisesGain,false,0);

                                MidiKeyState[note]=0;
                            }
                            break;
                            }
                        }
                        else
                        {
                            type=MidiKeyState[note]*5/128;

                            if(note<HEAD_BASE_NOTE&&type>1) type=1;

                            switch(type)
                            {
                            case 0:    //just head step, not pitched
                            {
                                FloppyStep(note%80);
                            }
                            break;

                            case 1:    //repeating slow steps with a pitch
                            {
                                if(!prev_any) FDD.low_freq_acc=1.0f;    //trigger first step right away

                                FDD.low_freq_add=(440.0f*pow(2.0f,(note-69-24)/12.0f))/sampleRate;

                                reset_low_freq=false;
                            }
                            break;

                            case 2:    //head buzz
                            {
                                if(note>=HEAD_BASE_NOTE&&note<HEAD_BASE_NOTE+HEAD_BUZZ_RANGE)
                                {
                                    FloppyStartHeadSample(&SampleHeadBuzz[note-HEAD_BASE_NOTE],pHeadBuzzGain,true,0);
                                }
                            }
                            break;

                            case 3:    //head seek from last position
                            case 4:    //head seek from initial position
                            {
                                if(note>=HEAD_BASE_NOTE&&note<HEAD_BASE_NOTE+HEAD_SEEK_RANGE)
                                {
                                    FloppyStartHeadSample(&SampleHeadSeek[note-HEAD_BASE_NOTE],pHeadSeekGain,true,type==4?0:FDD.head_sample_relative_ptr);
                                }
                            }
                            break;
                            }

                            head_stop=false;

                            break;
                        }
                    }
                }

                FloppySpindle(!spindle_stop);

                if(reset_low_freq)
                {
                    FDD.low_freq_acc=0;
                    FDD.low_freq_add=0;
                }

                if(head_stop)
                {
                    FDD.low_freq_acc=0;
                    FDD.low_freq_add=0;

                    FDD.head_sample_loop=false;
                }
            }
            break;

            case 0xb0:
                if (msg[1] >= 0x7b) //all notes off and mono/poly mode changes that also requires to do all notes off
                    memset(MidiKeyState,0,sizeof(MidiKeyState));
                break;
            }
        }

        FDD.low_freq_acc+=FDD.low_freq_add;

        if(FDD.low_freq_acc>=1.0f)
        {
            while(FDD.low_freq_acc>=1.0f) FDD.low_freq_acc-=1.0f;

            FloppyStep(-1);
        }

        level=0;

        if(FDD.spindle_sample)
        {
            level+=SampleRead(&FDD.spindle_sample,FDD.spindle_sample_ptr)*pSpindleGain;

            FDD.spindle_sample_ptr+=sample_step;

            if(FDD.spindle_enable)
            {
                if(FDD.spindle_sample_ptr>=FDD.spindle_sample.loop_end)
                {
                    FDD.spindle_sample_ptr-=(FDD.spindle_sample.loop_end-FDD.spindle_sample.loop_start);
                }
            }
            else
            {
                if(FDD.spindle_sample_ptr<FDD.spindle_sample.loop_end)
                {
                    FDD.spindle_sample_ptr=FDD.spindle_sample.loop_end;
                }
            }

            if(FDD.spindle_sample_ptr>FDD.spindle_sample.length-1) FDD.spindle_sample_ptr=FDD.spindle_sample.length-1;
        }

        if(FDD.head_sample)
        {
            level+=SampleRead(FDD.head_sample,FDD.head_sample_ptr)*FDD.head_gain;

            FDD.head_sample_ptr+=sample_step;

            if(FDD.head_sample_loop)
            {
                if(FDD.head_sample_ptr>=FDD.head_sample->loop_end)
                {
                    FDD.head_sample_ptr-=(FDD.head_sample->loop_end-FDD.head_sample->loop_start);
                }

                if(FDD.head_sample_ptr<FDD.head_sample->loop_start)
                {
                    FDD.head_sample_relative_ptr=.5f+.5f/FDD.head_sample->loop_start*(float)FDD.head_sample_ptr;
                }
                else
                {
                    FDD.head_sample_relative_ptr=1.0f/(FDD.head_sample->loop_end-FDD.head_sample->loop_start)*(float)FDD.head_sample_ptr;
                }

                FDD.head_sample_relative_ptr=floorf(FDD.head_sample_relative_ptr*160.0f)/160.0f;    //attempt to make rough steps, somewhat aligned to head clicks
            }
            else
            {
                if(FDD.head_sample_ptr<FDD.head_sample->loop_end)
                {
                    FDD.head_sample_ptr=FDD.head_sample->loop_end;
                }
            }

            if(FDD.head_sample_ptr>=FDD.head_sample->length)
            {
                FDD.head_sample=NULL;
            }
        }

        level=level*2.0f*pOutputGain;    //samples were too quiet comparing to other plugins, difficult to normalize without losing relative volumes

        (*outL++)=level;
        (*outR++)=level;
    }
}

bool FlopsterPlugin::MidiIsAnyKeyDown()
{
    int i;

    for(i=0;i<128;++i) if(MidiKeyState[i]) return true;

    return false;
}

float FlopsterPlugin::SampleRead(const sampleObject *sample, double pos)
{
    double s1,s2,fr;
    int ptr;

    if(!sample||!sample->wave) return 0;

    ptr=(int)pos;

    fr=(pos-(double)ptr);

    s1=sample->wave[ptr]/65536.0;

    ++ptr;

    if(ptr<sample->length) s2=sample->wave[ptr]/65536.0; else s2=s1;

    return (float)(s1+(s2-s1)*fr);
}

void FlopsterPlugin::FloppyStartHeadSample(const sampleObject *sample,float gain,bool loop,float relative)
{
    FDD.head_sample=sample;

    FDD.head_sample_loop=loop;
    FDD.head_gain=gain;

    if(relative==0)
    {
        FDD.head_sample_ptr=0;
    }
    else
    {
        FDD.head_sample_ptr=FDD.head_sample->loop_start+relative*(FDD.head_sample->loop_end-FDD.head_sample->loop_start);
    }
}

void FlopsterPlugin::FloppyStep(int pos)
{
    if(pos>=0)
    {
        FDD.head_pos=pos;
        FDD.head_dir=1;
    }

    FloppyStartHeadSample(&SampleHeadStep[FDD.head_pos],pHeadStepGain,false,0);

    FDD.head_pos+=FDD.head_dir;

    if(FDD.head_pos<=0)
    {
        FDD.head_pos=0;
        FDD.head_dir=-FDD.head_dir;
    }

    if(FDD.head_pos>=(STEP_SAMPLES_ALL-1))
    {
        FDD.head_pos=(STEP_SAMPLES_ALL-1);
        FDD.head_dir=-FDD.head_dir;
    }
}

void FlopsterPlugin::FloppySpindle(bool enable)
{
    if(FDD.spindle_enable==enable) return;

    FDD.spindle_enable=enable;

    if(enable&&FDD.spindle_sample_ptr>=FDD.spindle_sample.loop_end) FDD.spindle_sample_ptr=0;  //restart if it stopping
}

///
namespace DISTRHO {

Plugin *createPlugin()
{
    return new FlopsterPlugin;
}

} // namespace DISTRHO
