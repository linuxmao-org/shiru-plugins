#pragma once
#include "BasicWidget.hpp"
#include <functional>

class TriggerButton : public BasicWidget {
public:
    explicit TriggerButton(Widget *group);

    bool onMouse(const MouseEvent &event) override;
    void onDisplay() override;

    std::function<void()> TriggeredCallback;

private:
    bool fIsPressed = false;
};
