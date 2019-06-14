#pragma once
#include "Color.hpp"
#include <memory>

struct ColorPalette {
    ColorRGBA window_bg;
    ColorRGBA textedit_bg;
    ColorRGBA textedit_text;
    ColorRGBA textedit_cursor;
    ColorRGBA textedit_frame;
    ColorRGBA textlabel_text;
    ColorRGBA valuefill_frame;
    ColorRGBA valuefill_fill;
    ColorRGBA valuefill_text;

    virtual ~ColorPalette() {}
    static ColorPalette &getDefault();
};
