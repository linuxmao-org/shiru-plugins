#pragma once
#include "BasicWidget.hpp"
#include "Pango.hpp"
#include <string>
#include <memory>
#include <functional>

class TextEdit : public BasicWidget {
public:
    explicit TextEdit(Widget *group);

    const std::string &text() const noexcept { return fText; }
    void setText(const std::string &text);

    void setAllowMultiline(bool allow);
    void setHasKeyFocus(bool has);
    void setFont(const std::string &font);

    void onDisplay() override;
    bool onKeyboard(const KeyboardEvent &event) override;
    bool onSpecial(const SpecialEvent &event) override;
    bool onMouse(const MouseEvent &event) override;

    std::function<void(const std::string &)> ValueChangedCallback;

private:
    std::string fText;
    int fBytePos = 0;
    int fTrailing = 0;
    bool fAllowMultiline = false;
    bool fHasKeyFocus = false;
    std::string fFont;
    PangoLayout_u fTextLayout;
};
