#include "BitdrivePlugin.hpp"
#include <cstring>

BitdrivePlugin::BitdrivePlugin()
    : Plugin(Parameter_Count, DISTRHO_PLUGIN_NUM_PROGRAMS, State_Count)
{
    for (unsigned p = 0; p < Parameter_Count; ++p) {
        Parameter param;
        initParameter(p, param);
        setParameterValue(p, param.ranges.def);
    }
}

const char *BitdrivePlugin::getLabel() const
{
    return DISTRHO_PLUGIN_LABEL;
}

const char *BitdrivePlugin::getMaker() const
{
    return DISTRHO_PLUGIN_MAKER;
}

const char *BitdrivePlugin::getLicense() const
{
    return DISTRHO_PLUGIN_LICENSE;
}

const char *BitdrivePlugin::getDescription() const
{
    return DISTRHO_PLUGIN_DESCRIPTION;
}

const char *BitdrivePlugin::getHomePage() const
{
    return DISTRHO_PLUGIN_HOMEPAGE;
}

uint32_t BitdrivePlugin::getVersion() const
{
    return d_version(DISTRHO_PLUGIN_VERSION);
}

int64_t BitdrivePlugin::getUniqueId() const
{
    return d_cconst(DISTRHO_PLUGIN_UNIQUE_ID);
}

void BitdrivePlugin::initParameter(uint32_t index, Parameter &parameter)
{
    parameter.hints = kParameterIsAutomable;

    switch (index) {
    case pIdBypass:
        parameter.designation = kParameterDesignationBypass;
        break;
    case pIdInputGain:
        parameter.symbol = "InputGain";
        parameter.name = "Input Gain";
        parameter.ranges = ParameterRanges(0.5, 0.1, 1.0);
        parameter.hints |= kParameterIsLogarithmic;
        break;
    case pIdThreshold:
        parameter.symbol = "Threshold";
        parameter.name = "Threshold";
        parameter.ranges = ParameterRanges(0.0, -1.0, +1.0);
        break;
    case pIdOutputGain:
        parameter.symbol = "OutputGain";
        parameter.name = "Output Gain";
        parameter.ranges = ParameterRanges(1.0, 0.1, 1.0);
        parameter.hints |= kParameterIsLogarithmic;
        break;
    default:
        DISTRHO_SAFE_ASSERT(false);
    }
}

float BitdrivePlugin::getParameterValue(uint32_t index) const
{
    switch (index) {
    case pIdBypass:
        return pBypass;
    case pIdInputGain:
        return pInputGain;
    case pIdThreshold:
        return pThreshold;
    case pIdOutputGain:
        return pOutputGain;
    default:
        DISTRHO_SAFE_ASSERT_RETURN(false, 0);
    }
}

void BitdrivePlugin::setParameterValue(uint32_t index, float value)
{
    switch (index) {
    case pIdBypass:
        pBypass = value > 0.5f; break;
    case pIdInputGain:
        pInputGain = value; break;
    case pIdThreshold:
        pThreshold = value; break;
    case pIdOutputGain:
        pOutputGain = value; break;
    default:
        DISTRHO_SAFE_ASSERT_RETURN(false,);
    }
}

void BitdrivePlugin::run(const float **inputs, float **outputs, uint32_t frames)
{
    const float *inL =inputs[0];
    const float *inR =inputs[1];
    float *outL=outputs[0];
    float *outR=outputs[1];

    if (pBypass) {
        memcpy(outL, inL, frames * sizeof(float));
        memcpy(outR, inR, frames * sizeof(float));
        return;
    }

    float level,threshold;

    threshold=pThreshold/16.0f;

    for(uint32_t i = 0; i < frames; ++i)
    {
        level=inL[i]*pInputGain;

        if(threshold<0)
        {
            if(level<threshold) level=-pOutputGain; else level=0;
        }
        else
        {
            if(level>threshold) level=pOutputGain; else level=0;
        }

        outL[i]=level;

        level=inR[i]*pInputGain;

        if(threshold<0)
        {
            if(level<threshold) level=-pOutputGain; else level=0;
        }
        else
        {
            if(level>threshold) level=pOutputGain; else level=0;
        }

        outR[i]=level;
    }
}

///
namespace DISTRHO {

Plugin *createPlugin()
{
    return new BitdrivePlugin;
}

} // namespace DISTRHO
