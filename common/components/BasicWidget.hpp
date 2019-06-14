#pragma once
#include "Widget.hpp"
class ColorPalette;

class BasicWidget : public DGL::Widget {
public:
    explicit BasicWidget(Widget *group);
    virtual ~BasicWidget() {}

    const ColorPalette *getColorPalette() const noexcept { return fColorPalette; }
    void setColorPalette(const ColorPalette *p) noexcept { fColorPalette = p; }

    ///
    enum Orientation {
        Horizontal, Vertical
    };

private:
    const ColorPalette *fColorPalette = nullptr;
};
