#pragma once
#include "DistrhoUI.hpp"
#include <string>
#include <memory>
class TextEdit;
class TextLabel;
class ValueFill;

class EvaluaUI : public DISTRHO::UI
{
public:
    enum { GUI_WINDOW_WDT = 595, GUI_WINDOW_HGT = 240 };

    EvaluaUI();
    ~EvaluaUI();
    void parameterChanged(uint32_t index, float value) override;
    void programLoaded(uint32_t index) override;
    void stateChanged(const char *key, const char *value) override;
    void onDisplay() override;
    void uiIdle() override;

private:
    std::unique_ptr<TextEdit> fNameEdit;
    std::unique_ptr<TextEdit> fCodeEdit;
    std::unique_ptr<TextLabel> fStatusLabel;
    std::unique_ptr<TextLabel> fHintLabel;
    std::unique_ptr<ValueFill> fPolyValue;
    std::unique_ptr<ValueFill> fPortaValue;
    std::unique_ptr<ValueFill> fGainValue;
    std::string fLastCompiledCode;
};
