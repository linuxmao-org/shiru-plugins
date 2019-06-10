#pragma once
#include "DistrhoPlugin.hpp"
#include "EvaluaPresets.hpp"
#include <random>
#include <memory>
class EV;

class EvaluaPlugin : public DISTRHO::Plugin
{
public:
    EvaluaPlugin();

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

    enum { MAX_NAME_LEN = 32, MAX_PROGRAM_LEN = 1024 };

    struct ProgramObject {
        char Name[MAX_NAME_LEN + 1];
        char Data[MAX_PROGRAM_LEN + 1];
    };

    ProgramObject Program;

    int32_t PortaSpeed;  //0 slowest, 100 fastest
    int32_t Polyphony;  //max number of active channels, 1 for mono mode
    int32_t OutputGain;  //100 normal volume, <100 reduce, >100 amplify

    const char *Compile();

    struct ChannelObject {
        int32_t note;
        int32_t velocity;
        int64_t time_cnt;
        int64_t pitch_cnt;

        double time_acc;
        double pitch_acc;

        double freq;
        double freq_new;
    };

    unsigned char MidiKeyState[128];

    double MidiPitchBend;
    double MidiPitchBendRange;

    int64_t MidiModulationDepth;

    int32_t MidiRPNLSB;
    int32_t MidiRPNMSB;
    int32_t MidiDataLSB;
    int32_t MidiDataMSB;

    double SlideStep;

    std::unique_ptr<EV> ev;

    std::minstd_rand RandomNumberGenerator;

    ChannelObject SynthChannel[MaxPolyphony];

    int32_t SynthAllocateVoice(int32_t midi_ch, int32_t note);
    void SynthChannelChangeNote(int32_t chn, int32_t midi_ch, int32_t note, int32_t velocity);
    void SynthChannelReleaseNote(int32_t midi_ch, int32_t note);
};
