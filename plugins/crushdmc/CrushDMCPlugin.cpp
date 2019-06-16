#include "CrushDMCPlugin.hpp"
#include <cstring>

CrushDMCPlugin::CrushDMCPlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count)
{
    fBypassed = false;

    pInputGain=1.0f;
    pSampleRate=31;
    pOutputGain=1.0f;

    sSampleIn=0;
    sSampleAcc=0;
    sOutputCurrent=0;
    sSilence=0;

    for(unsigned i=0;i<4;++i) sOutBuf[i]=0;
}

const char *CrushDMCPlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *CrushDMCPlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *CrushDMCPlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *CrushDMCPlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *CrushDMCPlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t CrushDMCPlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t CrushDMCPlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void CrushDMCPlugin::initParameter(uint32_t index, Parameter &parameter)
{
    if (index == pIdBypass) {
        parameter.designation = kParameterDesignationBypass;
        return;
    }

    std::unique_ptr<ParameterEnumerationValue[]> pev;

    switch (index) {
    case pIdInputGain:
        parameter.symbol = "GAIN";
        parameter.name = "Input Gain";
        parameter.ranges.min = 0.1;
        parameter.ranges.max = 2;
        parameter.ranges.def = 1;
        parameter.hints = kParameterIsAutomable|kParameterIsLogarithmic;
        break;
    case pIdSampleRate:
        parameter.symbol = "SPFR";
        parameter.name = "Sample Rate";
        parameter.ranges.min = 0;
        parameter.ranges.max = 31;
        parameter.ranges.def = 31;
        parameter.hints = kParameterIsAutomable|kParameterIsInteger;
        pev.reset(new ParameterEnumerationValue[32]);
        for (unsigned i = 0; i < 32; ++i) {
            char str[256];
            sprintf(str,"%s $%X - %5.2f",(i<16?"PAL":"NTSC"),(i&15),sSampleRate[i]);
            pev[i].value = i;
            pev[i].label = str;
        }
        parameter.enumValues.values = pev.release();
        parameter.enumValues.count = 32;
        break;
    case pIdOutputGain:
        parameter.symbol = "GOUT";
        parameter.name = "Output Gain";
        parameter.ranges.min = 0.1;
        parameter.ranges.max = 2;
        parameter.ranges.def = 1;
        parameter.hints = kParameterIsAutomable|kParameterIsLogarithmic;
        break;
    default:
        DISTRHO_SAFE_ASSERT_RETURN(false,);
    }
}

float CrushDMCPlugin::getParameterValue(uint32_t index) const
{
    switch (index) {
    case pIdBypass:
        return fBypassed;
    case pIdInputGain:
        return pInputGain;
    case pIdSampleRate:
        return pSampleRate;
    case pIdOutputGain:
        return pOutputGain;
    default:
        DISTRHO_SAFE_ASSERT_RETURN(false, 0);
    }
}

void CrushDMCPlugin::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case pIdBypass:
        fBypassed = value > 0.5f;
        break;
    case pIdInputGain:
        pInputGain = value;
        break;
    case pIdSampleRate:
        pSampleRate = std::max<int32_t>(0, std::min<int32_t>(31, value));
        break;
    case pIdOutputGain:
        pOutputGain = value;
        break;
    default:
        DISTRHO_SAFE_ASSERT_RETURN(false,);
    }
}

void CrushDMCPlugin::run(const float **inputs, float **outputs, uint32_t frames)
{
    const float *in = inputs[0];
    float *out = outputs[0];

    if (fBypassed) {
        memcpy(out, in, frames * sizeof(float));
        return;
    }

    float f,level;
    double sample_rate,new_rate;
    int indac,max_silence;

    sample_rate=getSampleRate();
    new_rate=sSampleRate[pSampleRate];

    float sSampleMax=(float)(sample_rate/new_rate);

    max_silence=(int)(new_rate/50.0);

    for (uint32_t frame_index = 0; frame_index < frames; ++frame_index)
    {
        level=in[frame_index]*pInputGain;

        sSampleIn+=level;
        sSampleAcc+=1.0f;

        if(sSampleAcc>=sSampleMax)
        {
            level=sSampleIn/sSampleAcc;

            sSampleAcc-=sSampleMax;
            sSampleIn=0;

            indac=(int)(level*64.0f);

            if(sOutputCurrent<indac)
            {
                if(sOutputCurrent<62) sOutputCurrent+=2;
            }
            else
            {
                if(sOutputCurrent>=-62) sOutputCurrent-=2;
            }

            if(!(indac>=-2&&indac<=2))    //silence theshold
            {
                sSilence=0;
            }
            else
            {
                if(sSilence<max_silence) ++sSilence; else sOutputCurrent=0;    //reset output current
            }

            sOutBuf[3]=sOutBuf[2];
            sOutBuf[2]=sOutBuf[1];
            sOutBuf[1]=sOutBuf[0];
            sOutBuf[0]=((float)sOutputCurrent/64.0f);
        }

        f=sSampleAcc/sSampleMax;

        if(f>1.0f) f=1.0f;

        level=hermite4(f,sOutBuf[0],sOutBuf[1],sOutBuf[2],sOutBuf[3]);

        level=level*pOutputGain;

        out[frame_index]=level;
    }
}

///
namespace DISTRHO {

Plugin *createPlugin()
{
    return new CrushDMCPlugin;
}

} // namespace DISTRHO
