#pragma once
#include "Cairo.hpp"
#include <cstdint>

struct ColorRGBA {
    uint8_t r, g, b, a;
};

inline void cairo_set_source_color(cairo_t *cr, const ColorRGBA &c)
{
    double k = 1. / 255.;
    cairo_set_source_rgba(cr, k * c.r, k * c.g, k * c.b, k * c.a);
}
