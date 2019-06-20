#pragma once
#include "DistrhoPlugin.hpp"
#include "ChipWavePresets.hpp"
#include "ChipWaveShared.hpp"
#include <random>

class ChipWavePlugin : public DISTRHO::Plugin
{
public:
    ChipWavePlugin();

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
    int32_t SynthAllocateVoice(int32_t note);
    void SynthChannelChangeNote(int32_t chn, int32_t note);
    void SynthRestartEnvelope(int32_t chn);
    void SynthStopEnvelope(int32_t chn);
    void SynthAdvanceEnvelopes();

    bool MidiIsAnyKeyDown(void);

private:
    enum {
        MaxSynthChannels = 8
    };

    enum {
        eStageReset,
        eStageAttack,
        eStageDecay,
        eStageSustain,
        eStageRelease
    };

    struct SynthChannelObject {
        int32_t note;

        float velocity;

        float freq;
        float freq_new;

        SynthOscObject osca;
        SynthOscObject oscb;

        int32_t ev_stage;
        float ev_level;
        float ev_delta;

        int32_t ef_stage;
        float ef_level;
        float ef_delta;

        float volume;

        float slide_delay;
        float slide_osca;
        float slide_oscb;

        float lfo_count;
        float lfo_out;

        double filter_resofreq;
        double filter_amp;
        double filter_r;
        double filter_c;
        double filter_vibrapos;
        double filter_vibraspeed;
    };

private:
    std::minstd_rand RandomNumberGenerator;

    struct programObject {
        char name[MaxNameLen + 1];
        float values[Parameter_Count];
    };

    programObject Program;

    unsigned char MidiKeyState[128];

    SynthChannelObject SynthChannel[MaxSynthChannels];

    float sEnvelopeDiv;
    float sSlideStep;

    float MidiPitchBend;
    float MidiPitchBendRange;
    float MidiModulationDepth;
    float MidiModulationCount;

    int32_t MidiRPNLSB;
    int32_t MidiRPNMSB;
    int32_t MidiDataLSB;
    int32_t MidiDataMSB;
};
