#pragma once
#include "BasicWidget.hpp"
#include "Pango.hpp"
#include <string>
#include <functional>

class ValueFill : public BasicWidget {
public:
    explicit ValueFill(Widget *group);

    double value() const noexcept { return fValue; }
    void setValue(double value);

    void setValueBounds(double v1, double v2);
    void setNumSteps(unsigned numSteps);
    void setOrientation(Orientation ori);

    void setFont(const std::string &font);

    bool onMouse(const MouseEvent &event) override;
    bool onMotion(const MotionEvent &event) override;
    bool onScroll(const ScrollEvent &event) override;
    void onDisplay() override;

    std::function<void(double)> ValueChangedCallback;
    std::function<std::string(double)> TextFormatCallback;

private:
    double clampToBounds(double value);

private:
    double fValue = 0;
    double fValueBound1 = 0, fValueBound2 = 1;
    unsigned fNumSteps = 100;
    Orientation fOrientation = Horizontal;
    bool fIsDragging = false;
    PangoLayout_u fTextLayout;
    std::string fFont;
};
