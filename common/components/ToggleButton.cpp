#include "ToggleButton.hpp"
#include "Window.hpp"
#include "Cairo.hpp"
#include "ColorPalette.hpp"

ToggleButton::ToggleButton(Widget *group)
    : BasicWidget(group)
{
}

void ToggleButton::setValue(bool value)
{
    if (fValue == value)
        return;

    fValue = value;
    if (ValueChangedCallback)
        ValueChangedCallback(value);
    repaint();
}

bool ToggleButton::onMouse(const MouseEvent &event)
{
    DGL::Size<uint> wsize = getSize();
    DGL::Point<int> mpos = event.pos;

    bool inside = mpos.getX() >= 0 && mpos.getY() >= 0 &&
        (unsigned)mpos.getX() < wsize.getWidth() && (unsigned)mpos.getY() < wsize.getHeight();

    if (event.press && event.button == 1 && inside) {
        fIsPressed = true;
        repaint();
        return true;
    }
    else if (!event.press && event.button == 1) {
        if (fIsPressed) {
            fIsPressed = false;
            if (inside)
                setValue(!fValue);
            repaint();
        }
    }

    return false;
}

void ToggleButton::onDisplay()
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    const ColorPalette *cp = getColorPalette();

    int w = getWidth();
    int h = getHeight();

    cairo_set_source_color(cr, cp->button_frame);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_stroke(cr);

    if (fValue ^ fIsPressed) {
        int xpad = 2;
        int ypad = 2;
        cairo_set_source_color(cr, cp->button_pressed);
        cairo_rectangle(cr, xpad, ypad, w - 2 * xpad, h - 2 * ypad);
        cairo_fill(cr);
    }
}
