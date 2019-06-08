#include "TextLabel.hpp"
#include "Window.hpp"
#include "../ColorPalette.hpp"

TextLabel::TextLabel(Widget *group)
    : Widget(group)
{
}

void TextLabel::setText(const std::string &text)
{
    if (fText == text)
        return;

    fText.assign(text);

    if (fTextLayout)
        pango_layout_set_text(fTextLayout.get(), text.data(), (int)text.size());

    repaint();
}

void TextLabel::setFont(const std::string &font)
{
    if (fFont == font)
        return;

    fFont = font;

    if (fTextLayout)
        pango_layout_set_font_description(fTextLayout.get(), font.empty() ? nullptr : pango_font_description_from_string(font.c_str()));

    repaint();
}

void TextLabel::onDisplay()
{
    cairo_t *cr = getParentWindow().getGraphicsContext().cairo;

    PangoLayout *layout = fTextLayout.get();
    if (!layout) {
        layout = pango_cairo_create_layout(cr);
        fTextLayout.reset(layout);
        pango_layout_set_text(layout, fText.data(), (int)fText.size());
        pango_layout_set_font_description(fTextLayout.get(), fFont.empty() ? nullptr : pango_font_description_from_string(fFont.c_str()));
    }

    cairo_set_source_color(cr, ColorPalette::textlabel_text);
    pango_cairo_show_layout(cr, layout);

    if (false) {  // XXX: test only
        cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
        cairo_rectangle(cr, 0, 0, getWidth(), getHeight());
        cairo_stroke(cr);
    }
}
