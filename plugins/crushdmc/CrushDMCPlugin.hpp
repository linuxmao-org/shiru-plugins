#pragma once
#include "DistrhoPlugin.hpp"
#include "CrushDMCShared.hpp"
#include <memory>
#include <cstdint>

class CrushDMCPlugin : public DISTRHO::Plugin {
public:
    CrushDMCPlugin();

    const char *getLabel() const override;
    const char *getMaker() const override;
    const char *getLicense() const override;
    const char *getDescription() const override;
    const char *getHomePage() const override;
    uint32_t getVersion() const override;
    int64_t getUniqueId() const override;

    void initParameter(uint32_t index, Parameter &parameter) override;
    float getParameterValue(uint32_t index) const override;
    void setParameterValue(uint32_t index, float value) override;

    void run(const float **inputs, float **outputs, uint32_t frames) override;

private:
    bool fBypassed;

    float pInputGain;
    unsigned pSampleRate;
    float pOutputGain;

    float sSampleIn;
    float sSampleAcc;
    //float sSampleMax;
    float sOutBuf[4];

    int32_t sOutputCurrent;
    int32_t sSilence;
};
