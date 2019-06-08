#pragma once
#include "Cairo.hpp"
#include <cstdint>

struct ColorRGBA {
    uint8_t r, g, b, a;
};

struct ColorPalette {
    static const ColorRGBA window_bg;
    static const ColorRGBA textedit_bg;
    static const ColorRGBA textedit_text;
    static const ColorRGBA textedit_cursor;
    static const ColorRGBA textedit_frame;
    static const ColorRGBA textlabel_text;
    static const ColorRGBA valuefill_frame;
    static const ColorRGBA valuefill_fill;
    static const ColorRGBA valuefill_text;
};

inline void cairo_set_source_color(cairo_t *cr, const ColorRGBA &c)
{
    double k = 1. / 255.;
    cairo_set_source_rgba(cr, k * c.r, k * c.g, k * c.b, k * c.a);
}
