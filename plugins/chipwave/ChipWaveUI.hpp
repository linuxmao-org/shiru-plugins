#pragma once
#include "DistrhoUI.hpp"
#include "Image.hpp"
#include <vector>
#include <memory>
class TextEdit;

class ChipWaveUI : public DISTRHO::UI
{
public:
    ChipWaveUI();

    bool onMotion(const MotionEvent &event) override;
    void onDisplay() override;
    void parameterChanged(uint32_t index, float value) override;
    void programLoaded(uint32_t index) override;
    void stateChanged(const char* key, const char* value) override;

private:
    void SliderAdd(int32_t x, int32_t y, int32_t w, int32_t h, int32_t param, int32_t steps, bool invert);
    void RenderWaveform(int32_t x, int32_t y, int32_t w, int32_t h, int32_t osc);
    void RenderEnvelope(int32_t x, int32_t y, int32_t w, int32_t h, int32_t env);

private:
    float getControlValue(uint32_t index) const;
    void setControlValue(uint32_t index, float value);

private:
    bool fGraphicsInitialized = false;
    DGL::Image fBackgroundImage;
    std::unique_ptr<TextEdit> fNameEdit;
    std::unique_ptr<std::unique_ptr<DGL::Widget>[]> fControls;
    int fControlHovered = -1;
    int32_t fNoise[65536];
};
