#include "EvaluaUI.hpp"
#include "EvaluaPresets.hpp"
#include "Window.hpp"
#include "common/components/ColorPalette.hpp"
#include "common/components/TextEdit.hpp"
#include "common/components/TextLabel.hpp"
#include "common/components/ValueFill.hpp"
#include "ev.h"
#include <cmath>
#include <cstdio>
#include <cstring>

EvaluaUI::EvaluaUI()
    : UI(GUI_WINDOW_WDT, GUI_WINDOW_HGT)
{
    TextEdit *nameEdit = new TextEdit(this);
    fNameEdit.reset(nameEdit);
    nameEdit->ValueChangedCallback =
        [this](const std::string &value) { setState("ProgramName", value.c_str()); };
    nameEdit->setSize(200, 20);
    nameEdit->setAbsolutePos(100, 10);

    TextEdit *codeEdit = new TextEdit(this);
    fCodeEdit.reset(codeEdit);
    codeEdit->setAllowMultiline(true);
    codeEdit->setSize(580, 125);
    codeEdit->setAbsolutePos(8, 35);

    TextLabel *statusLabel = new TextLabel(this);
    fStatusLabel.reset(statusLabel);
    statusLabel->setFont("Sans 8.5");
    statusLabel->setAbsolutePos(10, 10);
    statusLabel->setSize(80, 20);

    DGL::Point<int> hintPos = fCodeEdit->getAbsolutePos();
    hintPos.setY(hintPos.getY() + fCodeEdit->getHeight() + 8);

    TextLabel *hintLabel = new TextLabel(this);
    fHintLabel.reset(hintLabel);
    hintLabel->setFont("Sans 8.5");
    hintLabel->setText("DEC and HEX(with 0x prefix) integer numbers allowed\n"
                       "+ - * / % & | ^ << >> ( ) operators allowed (ADD,SUB,MUL,DIV,MOD,AND,OR,XOR,SHL,SHR)\n"
                       "P=pitch, depends on note, considers 256 per period; T=time since note start (65536 per second)\n"
                       "V=velocity 0..127; M=modulation wheel 0..127; R=random 0..32767");
    hintLabel->setAbsolutePos(hintPos.getX(), hintPos.getY());
    hintLabel->setSize(580, 60);

    ValueFill *polyValue = new ValueFill(this);
    fPolyValue.reset(polyValue);
    polyValue->setValueBounds(MinPolyphony, MaxPolyphony);
    polyValue->setNumSteps(MaxPolyphony - MinPolyphony);
    polyValue->setFont("Sans 8.5");
    polyValue->ValueChangedCallback =
        [this](double value) { setState("Polyphony", std::to_string(lround(value)).c_str()); };
    polyValue->TextFormatCallback =
        [](double value) -> std::string {
            long val = lround(value);
            if (val == 1) return "Mono";
            return "Poly:" + std::to_string(val);
        };
    polyValue->setAbsolutePos(310, 10);
    polyValue->setSize(64, 20);

    ValueFill *portaValue = new ValueFill(this);
    fPortaValue.reset(portaValue);
    portaValue->setValueBounds(MinPortaSpeed, MaxPortaSpeed);
    portaValue->setNumSteps(MaxPortaSpeed - MinPortaSpeed);
    portaValue->setFont("Sans 8.5");
    portaValue->ValueChangedCallback =
        [this](double value) { setState("PortaSpeed", std::to_string(lround(value)).c_str()); };
    portaValue->TextFormatCallback =
        [](double value) -> std::string { return "Porta:" + std::to_string(lround(value)); };
    portaValue->setAbsolutePos(384, 10);
    portaValue->setSize(64, 20);

    ValueFill *gainValue = new ValueFill(this);
    fGainValue.reset(gainValue);
    gainValue->setValueBounds(MinOutputGain, MaxOutputGain);
    gainValue->setNumSteps(MaxOutputGain - MinOutputGain);
    gainValue->setFont("Sans 8.5");
    gainValue->ValueChangedCallback =
        [this](double value) { setState("OutputGain", std::to_string(lround(value)).c_str()); };
    gainValue->TextFormatCallback =
        [](double value) -> std::string { return "Gain:" + std::to_string(lround(value)); };
    gainValue->setAbsolutePos(458, 10);
    gainValue->setSize(64, 20);
}

EvaluaUI::~EvaluaUI()
{
}

void EvaluaUI::parameterChanged(uint32_t index, float value)
{
    (void)index;
    (void)value;
}

void EvaluaUI::programLoaded(uint32_t index)
{
    char namebuf[32];
    const char *name;
    const char *code;

    if (index < PresetDataLength) {
        name = PresetData[index].first;
        code = PresetData[index].second;
    }
    else {
        sprintf(namebuf, "%3.3u default", index);
        name = namebuf;
        code = PresetData[0].second;
    }

    fNameEdit->setText(name);
    fCodeEdit->setText(code);

    fPolyValue->setValue(PresetPolyphony);
    fPortaValue->setValue(PresetPortaSpeed);
    fGainValue->setValue(PresetOuputGain);
}

void EvaluaUI::stateChanged(const char *key, const char *value)
{
    if (!strcmp(key, "ProgramName"))
        fNameEdit->setText(value);
    else if (!strcmp(key, "ProgramData"))
        fCodeEdit->setText(value);
    else if (!strcmp(key, "Polyphony"))
        fPolyValue->setValue(strtol(value, nullptr, 0));
    else if (!strcmp(key, "PortaSpeed"))
        fPortaValue->setValue(strtol(value, nullptr, 0));
    else if (!strcmp(key, "OutputGain"))
        fGainValue->setValue(strtol(value, nullptr, 0));
}

void EvaluaUI::onDisplay()
{
    cairo_t *cr = getParentWindow().getGraphicsContext().cairo;
    const ColorPalette &cp = ColorPalette::getDefault();

    DGL::Size<uint> sz = getSize();
    int w = sz.getWidth();
    int h = sz.getHeight();

    cairo_rectangle(cr, 0, 0, w, h);
    cairo_set_source_color(cr, cp.window_bg);
    cairo_fill(cr);
}

void EvaluaUI::uiIdle()
{
    const std::string &code = fCodeEdit->text();

    if (code != fLastCompiledCode) {
        EV ev;
        const char *err = ev.Parse(code.c_str());
        fStatusLabel->setText(err ? err : "OK");
        fLastCompiledCode = code;
        if (!err)
            setState("ProgramData", code.c_str());
    }
}

///
namespace DISTRHO {

UI *createUI()
{
    return new EvaluaUI;
}

} // namespace DISTRHO
