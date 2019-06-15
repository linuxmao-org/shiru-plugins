#pragma once
#include "BasicWidget.hpp"
#include "Pango.hpp"
#include <string>
#include <memory>

class TextLabel : public BasicWidget {
public:
    explicit TextLabel(Widget *group);

    const std::string &text() const noexcept { return fText; }
    void setText(const std::string &text);

    void setFont(const std::string &font);

    void onDisplay() override;

private:
    std::string fText;
    std::string fFont;
    PangoLayout_u fTextLayout;
};
