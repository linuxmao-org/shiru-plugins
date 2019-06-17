#pragma once
#include "ChipDrumShared.hpp"
#include "DistrhoUI.hpp"
#include "Image.hpp"
#include <vector>
#include <memory>
class TextEdit;
class ToggleButton;
class TriggerButton;

class ChipDrumUI : public DISTRHO::UI
{
public:
    ChipDrumUI();
    ~ChipDrumUI();

    bool onMotion(const MotionEvent &event) override;
    void onDisplay() override;
    void parameterChanged(uint32_t index, float value) override;
    void programLoaded(uint32_t index) override;
    void stateChanged(const char* key, const char* value) override;

private:
    void ToggleButtonAdd(int32_t x, int32_t y, int32_t w, int32_t h, bool hover);
    void TriggerButtonAdd(int32_t x, int32_t y, int32_t w, int32_t h, bool hover);
    void SliderAdd(int32_t x, int32_t y, int32_t w, int32_t h, int32_t param, int32_t steps, bool invert);
    void RenderWaveform(int32_t x, int32_t y, int32_t w, int32_t h);
    void RenderEnvelope(int32_t x, int32_t y, int32_t w, int32_t h);

private:
    void CopyDrum();
    void PasteDrum();

private:
    float getControlValue(uint32_t index) const;
    void setControlValue(uint32_t index, float value);

    void selectNote(unsigned note);

private:
    bool fGraphicsInitialized = false;
    DGL::Image fBackgroundImage;
    std::unique_ptr<TextEdit> fNameEdit;
    std::unique_ptr<std::unique_ptr<DGL::Widget>[]> fControls;
    std::unique_ptr<int[]> fControlNumSteps;
    int fControlHovered = -1;
    unsigned fSelectedNoteNumber = 0;

    std::vector<std::unique_ptr<ToggleButton>> fToggleButtons;
    std::vector<std::unique_ptr<TriggerButton>> fTriggerButtons;
    ToggleButton *fNoteSelectButton[SYNTH_NOTES] = {};

    float fParameterValues[Parameter_Count] = {};
    Parameter fParameters[Parameter_Count];

    bool fCopyBufActive = false;
    float fCopyBuf[Parameter_Count] = {};
};
