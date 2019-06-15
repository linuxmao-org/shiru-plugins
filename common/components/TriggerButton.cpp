#include "TriggerButton.hpp"
#include "Window.hpp"
#include "Cairo.hpp"
#include "ColorPalette.hpp"

TriggerButton::TriggerButton(Widget *group)
    : BasicWidget(group)
{
}

bool TriggerButton::onMouse(const MouseEvent &event)
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
            if (inside && TriggeredCallback)
                TriggeredCallback();
            repaint();
        }
    }

    return false;
}

void TriggerButton::onDisplay()
{
    const DGL::GraphicsContext &gc = getParentWindow().getGraphicsContext();
    cairo_t *cr = gc.cairo;

    const ColorPalette *cp = getColorPalette();

    int w = getWidth();
    int h = getHeight();

    cairo_set_source_color(cr, cp->button_frame);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_stroke(cr);

    if (fIsPressed) {
        int xpad = 2;
        int ypad = 2;
        cairo_set_source_color(cr, cp->button_pressed);
        cairo_rectangle(cr, xpad, ypad, w - 2 * xpad, h - 2 * ypad);
        cairo_fill(cr);
    }
}
