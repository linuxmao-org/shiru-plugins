#pragma once
#include "BasicWidget.hpp"
#include <functional>

class ToggleButton : public BasicWidget {
public:
    explicit ToggleButton(Widget *group);

    bool value() const noexcept { return fValue; }
    void setValue(bool value);

    bool onMouse(const MouseEvent &event) override;
    void onDisplay() override;

    std::function<void(bool)> ValueChangedCallback;

private:
    bool fValue = false;
    bool fIsPressed = false;
};
