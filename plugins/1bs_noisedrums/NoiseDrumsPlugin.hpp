#pragma once
#include "DistrhoPlugin.hpp"
#include "NoiseDrumsShared.hpp"

class NoiseDrumsPlugin : public DISTRHO::Plugin {
public:
    NoiseDrumsPlugin();

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

    void initProgramName(uint32_t index, String &name) override;
    void loadProgram(uint32_t index) override;

    void initState(uint32_t index, String &state_key, String &default_value) override;
    String getState(const char *key) const override;
    void setState(const char *key, const char *value) override;

    void run(const float **inputs, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count) override;

private:
    static int FloatToNoisePeriod(float value);

    void SynthNewNoiseBurst(int32_t chn);

private:
    struct programObject {
        char name[MAX_NAME_LEN + 1];
        float values[Parameter_Count];
    };

    programObject Program;

    struct SynthChannelObject
    {
        int32_t note;
        int32_t burst;

        float duration;
        float accumulator;
        float increment;

        int32_t period;
        int32_t ptr;

        float volume;

        int32_t output;
    };

    SynthChannelObject SynthChannel[SYNTH_CHANNELS];

    unsigned Noise[2048];
};
