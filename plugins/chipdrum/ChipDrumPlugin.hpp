#pragma once
#include "DistrhoPlugin.hpp"
#include "ChipDrumPresets.hpp"
#include "ChipDrumShared.hpp"
#include <random>

class ChipDrumPlugin : public DISTRHO::Plugin
{
public:
    ChipDrumPlugin();

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

    void run(const float **, float **outputs, uint32_t frames, const MidiEvent *events, uint32_t event_count) override;

private:
    void SynthRestartTone(int32_t chn);
    void SynthRestartNoise(int32_t chn);

private:
    struct SynthChannelObject {
        int32_t note;
        int32_t notem;

        float velocity;

        float tone_sample;

        float tone_level;
        float tone_decay;
        float tone_sustain;
        float tone_release;

        float tone_acc;
        float tone_add;
        float tone_adda;
        float tone_delta;
        float tone_over;

        float tone_env_acc;
        float tone_env_add1;
        float tone_env_add2;

        int32_t tone_wave;

        float noise_sample;

        float noise_level;
        float noise_decay;
        float noise_sustain;
        float noise_release;

        float noise_env_acc;
        float noise_env_add1;
        float noise_env_add2;

        float noise_acc;
        float noise_add1;
        float noise_add2;
        float noise_add1a;
        float noise_add2a;

        float noise_frame_acc;
        float noise_frame_add1;
        float noise_frame_add2;

        int32_t noise_mask;
        int32_t noise_ptr;
        int32_t noise_seed;
        int32_t noise_type;

        float retrigger_acc;
        float retrigger_add;
        int32_t retrigger_count;
        int32_t retrigger_route;

        int32_t group;

        float bit_depth;

        int32_t filter_route;

        double lpf_resofreq;
        double lpf_r;
        double lpf_c;
        double lpf_vibrapos;
        double lpf_vibraspeed;

        double hpf_resofreq;
        double hpf_r;
        double hpf_c;
        double hpf_vibrapos;
        double hpf_vibraspeed;

        float frame_acc;
        float frame_add;

        float volume_l;
        float volume_r;
    };

private:
    std::minstd_rand RandomNumberGenerator;

    Preset Program;
    SynthChannelObject SynthChannel[SYNTH_CHANNELS];
};
