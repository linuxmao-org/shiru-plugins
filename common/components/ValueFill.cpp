#include "ValueFill.hpp"
#include "Window.hpp"
#include "ColorPalette.hpp"

ValueFill::ValueFill(Widget *group)
    : BasicWidget(group)
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

void ValueFill::setOrientation(Orientation ori)
{
    if (fOrientation == ori)
        return;

    fOrientation = ori;
    repaint();
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

    if (!fIsDragging && event.press && event.button == 1) {
        double fill = -1;

        if (fOrientation == Horizontal && mpos.getY() >= 0 && (unsigned)mpos.getY() < wsize.getHeight())
            fill = mpos.getX() / (double)wsize.getWidth();
        else if (fOrientation == Vertical && mpos.getX() >= 0 && (unsigned)mpos.getX() < wsize.getWidth())
            fill = 1.0 - (mpos.getY() / (double)wsize.getHeight());

        if (fill >= 0 && fill <= 1) {
            fIsDragging = true;
            setValue(fValueBound1 + fill * (fValueBound2 - fValueBound1));
            return true;
        }
    }
    else if (fIsDragging && !event.press && event.button == 1) {
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
        double fill;
        if (fOrientation == Horizontal)
            fill = mpos.getX() / (double)wsize.getWidth();
        else
            fill = 1.0 - (mpos.getY() / (double)wsize.getHeight());
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
        if (fOrientation == Vertical)
            amount = -amount;
        setValue(fValue + amount * (fValueBound2 - fValueBound1) / fNumSteps);
        return true;
    }

    return false;
}

void ValueFill::onDisplay()
{
    cairo_t *cr = getParentWindow().getGraphicsContext().cairo;
    const ColorPalette *cp = getColorPalette();

    int w = getWidth();
    int h = getHeight();

    cairo_rectangle(cr, 0, 0, w, h);
    cairo_set_source_color(cr, cp->valuefill_frame);
    cairo_stroke(cr);

    //
    double v1 = fValueBound1;
    double v2 = fValueBound2;

    //
    double value = fValue;
    double fill = 0;
    if (v1 != v2)
        fill = (value - v1) / (v2 - v1);

    //
    int xpad = 2, ypad = 2;
    if (fOrientation == Horizontal)
        cairo_rectangle(cr, xpad, ypad, fill * (w - 2 * xpad), h - 2 * ypad);
    else
        cairo_rectangle(cr, xpad, h - ypad, w - 2 * xpad, - fill * (h - 2 * xpad));
    cairo_set_source_color(cr, cp->valuefill_fill);
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

        cairo_set_source_color(cr, cp->valuefill_text);
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
