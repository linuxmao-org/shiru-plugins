#pragma once
#include "DistrhoPlugin.hpp"
#include "FlopsterShared.hpp"
#include "FlopsterSamples.hpp"

class FlopsterPlugin : public DISTRHO::Plugin
{
public:
    FlopsterPlugin();

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

    void initState(uint32_t index, String &state_key, String &default_value) override;
    String getState(const char *key) const override;
    void setState(const char *key, const char *value) override;

    void run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count) override;

private:
    bool MidiIsAnyKeyDown();

    float SampleRead(const sampleObject *sample, double pos);

    void FloppyStartHeadSample(const sampleObject *sample, float gain, bool loop, float relative);
    void FloppyStep(int pos);
    void FloppySpindle(bool enable);

private:
    float pHeadStepGain;
    float pHeadSeekGain;
    float pHeadBuzzGain;
    float pSpindleGain;
    float pNoisesGain;
    float pOutputGain;

    unsigned char MidiKeyState[128];

    sampleObject SampleHeadStep[STEP_SAMPLES_ALL];
    sampleObject SampleHeadBuzz[HEAD_BUZZ_RANGE];
    sampleObject SampleHeadSeek[HEAD_SEEK_RANGE];

    sampleObject SampleDiskPush;
    sampleObject SampleDiskInsert;
    sampleObject SampleDiskEject;
    sampleObject SampleDiskPull;

    struct
    {
        sampleObject spindle_sample;

        double spindle_sample_ptr;
        bool spindle_enable;

        const sampleObject *head_sample;

        double head_sample_ptr;
        bool   head_sample_loop;
        float  head_sample_relative_ptr;

        int    head_pos;
        int    head_dir;

        float  head_gain;

        float low_freq_acc;
        float low_freq_add;

    } FDD;
};
