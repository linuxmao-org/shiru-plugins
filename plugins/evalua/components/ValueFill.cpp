#include "ValueFill.hpp"
#include "Window.hpp"
#include "../ColorPalette.hpp"

ValueFill::ValueFill(Widget *group)
    : Widget(group)
{
}

void ValueFill::setValue(double value)
{
    value = clampToBounds(value);

    if (fValue == value)
        return;

    fValue = value;
    if (ValueChangedCallback)
        ValueChangedCallback(value);
    repaint();
}

void ValueFill::setValueBounds(double v1, double v2)
{
    fValueBound1 = v1;
    fValueBound2 = v2;
    setValue(fValue);
}

void ValueFill::setNumSteps(unsigned numSteps)
{
    fNumSteps = numSteps;
}

void ValueFill::setFont(const std::string &font)
{
    if (fFont == font)
        return;

    fFont = font;

    if (fTextLayout)
        pango_layout_set_font_description(fTextLayout.get(), font.empty() ? nullptr : pango_font_description_from_string(font.c_str()));

    repaint();
}

bool ValueFill::onMouse(const MouseEvent &event)
{
    DGL::Size<uint> wsize = getSize();
    DGL::Point<int> mpos = event.pos;

    if (event.press && event.button == 1) {
        if (mpos.getY() >= 0 && (unsigned)mpos.getY() < wsize.getHeight()) {
            double fill = mpos.getX() / (double)wsize.getWidth();
            if (fill >= 0 && fill <= 1) {
                fIsDragging = true;
                setValue(fValueBound1 + fill * (fValueBound2 - fValueBound1));
                return true;
            }
        }
    }
    else if (!event.press && event.button == 1) {
        fIsDragging = false;
        return true;
    }

    return false;
}

bool ValueFill::onMotion(const MotionEvent &event)
{
    DGL::Size<uint> wsize = getSize();
    DGL::Point<int> mpos = event.pos;

    if (fIsDragging) {
        double fill = mpos.getX() / (double)wsize.getWidth();
        fill = (fill < 0) ? 0 : fill;
        fill = (fill > 1) ? 1 : fill;
        setValue(fValueBound1 + fill * (fValueBound2 - fValueBound1));
        return true;
    }

    return false;
}

bool ValueFill::onScroll(const ScrollEvent &event)
{
    DGL::Size<uint> wsize = getSize();
    DGL::Point<int> mpos = event.pos;

    bool inside =
        mpos.getX() >= 0 && mpos.getY() >= 0 &&
        (unsigned)mpos.getX() < wsize.getWidth() && (unsigned)mpos.getY() < wsize.getHeight();

    if (inside) {
        double amount = event.delta.getX() - event.delta.getY();
        setValue(fValue + amount * (fValueBound2 - fValueBound1) / fNumSteps);
        return true;
    }

    return false;
}

void ValueFill::onDisplay()
{
    cairo_t *cr = getParentWindow().getGraphicsContext().cairo;

    int w = getWidth();
    int h = getHeight();

    cairo_rectangle(cr, 0, 0, w, h);
    cairo_set_source_color(cr, ColorPalette::valuefill_frame);
    cairo_stroke(cr);

    //
    double vmin = fValueBound1;
    double vmax = fValueBound2;
    if (vmin > vmax)
        std::swap(vmin, vmax);

    //
    double value = fValue;
    double fill = 0;
    if (vmax != vmin)
        fill = (value - vmin) / (vmax - vmin);

    //
    int xpad = 2, ypad = 2;
    cairo_rectangle(cr, xpad, ypad, fill * (w - 2 * xpad), h - 2 * ypad);
    cairo_set_source_color(cr, ColorPalette::valuefill_fill);
    cairo_fill(cr);

    //
    std::string text;
    if (TextFormatCallback)
        text = TextFormatCallback(value);
    if (!text.empty()) {
        PangoLayout *layout = fTextLayout.get();
        if (!layout) {
            layout = pango_cairo_create_layout(cr);
            fTextLayout.reset(layout);
            pango_layout_set_font_description(layout, fFont.empty() ? nullptr : pango_font_description_from_string(fFont.c_str()));
            pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);
        }
        pango_layout_set_width(layout, pango_units_from_double(getWidth()));
        pango_layout_set_text(layout, text.data(), (int)text.size());

        // vertical alignment
        PangoRectangle rect;
        pango_layout_get_extents(layout, nullptr, &rect);
        double texth = pango_units_to_double(rect.height);
        cairo_translate(cr, 0, 0.5 * (getHeight() - texth));

        cairo_set_source_color(cr, ColorPalette::valuefill_text);
        pango_cairo_show_layout(cr, layout);
    }
}

double ValueFill::clampToBounds(double value)
{
    double vmin = fValueBound1;
    double vmax = fValueBound2;
    if (vmin > vmax)
        std::swap(vmin, vmax);

    value = (value < vmin) ? vmin : value;
    value = (value > vmax) ? vmax : value;
    return value;
}
