#include "TextEdit.hpp"
#include "Cairo.hpp"
#include "Window.hpp"
#include "../ColorPalette.hpp"
#include <boost/locale/utf.hpp>

TextEdit::TextEdit(Widget *group)
    : Widget(group)
{
}

void TextEdit::setText(const std::string &text)
{
    if (fText == text)
        return;

    fText.assign(text);
    fBytePos = (int)text.size();
    fTrailing = 0;

    if (fTextLayout)
        pango_layout_set_text(fTextLayout.get(), text.data(), (int)text.size());

    if (ValueChangedCallback)
        ValueChangedCallback(fText);

    repaint();
}

void TextEdit::setAllowMultiline(bool allow)
{
    fAllowMultiline = allow;
}

void TextEdit::setHasKeyFocus(bool has)
{
    if (fHasKeyFocus == has)
        return;

    fHasKeyFocus = has;
    repaint();
}

void TextEdit::setFont(const std::string &font)
{
    if (fFont == font)
        return;

    fFont = font;

    if (fTextLayout)
        pango_layout_set_font_description(fTextLayout.get(), font.empty() ? nullptr : pango_font_description_from_string(font.c_str()));

    repaint();
}

void TextEdit::onDisplay()
{
    cairo_t *cr = getParentWindow().getGraphicsContext().cairo;

    PangoLayout *layout = fTextLayout.get();
    if (!layout) {
        layout = pango_cairo_create_layout(cr);
        fTextLayout.reset(layout);
        pango_layout_set_text(layout, fText.data(), fText.size());
        pango_layout_set_font_description(fTextLayout.get(), fFont.empty() ? nullptr : pango_font_description_from_string(fFont.c_str()));
    }

    DGL::Size<uint> sz = getSize();
    int w = sz.getWidth();
    int h = sz.getHeight();

    // paint the text box
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_set_source_color(cr, ColorPalette::textedit_bg);
    cairo_fill_preserve(cr);
    cairo_set_source_color(cr, ColorPalette::textedit_frame);
    cairo_stroke(cr);

    // allow cursor to be visible at the left
    cairo_translate(cr, 2, 0);

    // paint the cursor and text
    if (fHasKeyFocus) {
        PangoRectangle cursorRect = {};
        pango_layout_get_cursor_pos(layout, fBytePos + fTrailing, &cursorRect, nullptr);
        double cursx = pango_units_to_double(cursorRect.x);
        double cursy = pango_units_to_double(cursorRect.y);
        double cursh = pango_units_to_double(cursorRect.height);
        cairo_new_path(cr);
        cairo_move_to(cr, cursx, cursy);
        cairo_line_to(cr, cursx, cursy + cursh);
        cairo_set_source_color(cr, ColorPalette::textedit_cursor);
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
    }

    cairo_set_source_color(cr, ColorPalette::textedit_text);
    pango_cairo_show_layout(cr, layout);
}

bool TextEdit::onKeyboard(const KeyboardEvent &event)
{
    if (!fHasKeyFocus)
        return false;

    PangoLayout *layout = fTextLayout.get();
    unsigned key = event.key;

    if (event.press) {
        switch (key) {
        case '\b':
            if (layout) {
                int index = fBytePos + fTrailing;
                if (index > 0) {
                    pango_layout_move_cursor_visually(layout, true, fBytePos, fTrailing, -1, &fBytePos, &fTrailing);
                    int new_index = fBytePos + fTrailing;
                    fText.erase(fText.begin() + new_index, fText.begin() + index);
                    pango_layout_set_text(layout, fText.data(), (int)fText.size());
                    fBytePos = new_index;
                    fTrailing = 0;

                    if (ValueChangedCallback)
                        ValueChangedCallback(fText);

                    repaint();
                }
            }
            return true;
        case 0x7f:
            if (layout) {
                int index = fBytePos + fTrailing;
                if (index < (int)fText.size()) {
                    pango_layout_move_cursor_visually(layout, true, fBytePos, fTrailing, +1, &fBytePos, &fTrailing);
                    int new_index = fBytePos + fTrailing;
                    fText.erase(fText.begin() + index, fText.begin() + new_index);
                    pango_layout_set_text(layout, fText.data(), (int)fText.size());
                    fBytePos = index;
                    fTrailing = 0;

                    if (ValueChangedCallback)
                        ValueChangedCallback(fText);

                    repaint();
                }
            }
            return true;
        default:
            if (((key >= 0x20 && key < 0x7f) || key == '\t' ||
                 (fAllowMultiline && (key == '\r' || key == '\n')))) {
                if (key == '\r')
                    key = '\n';

                //TODO: non-ASCII keys
                int utf_size = 1;

                if (layout) {
                    int index = fBytePos + fTrailing;
                    fText.insert(fText.begin() + index, key);
                    pango_layout_set_text(layout, fText.data(), (int)fText.size());
                    fBytePos = index + utf_size;
                    fTrailing = 0;

                    if (ValueChangedCallback)
                        ValueChangedCallback(fText);

                    repaint();
                }
                return true;
            }
        }
    }

    return false;
}

bool TextEdit::onSpecial(const SpecialEvent &event)
{
    if (!fHasKeyFocus)
        return false;

    PangoLayout *layout = fTextLayout.get();

    if (event.press) {
        switch (event.key) {
        case DGL::kKeyLeft: {
            if (layout && fBytePos + fTrailing > 0) {
                pango_layout_move_cursor_visually(layout, true, fBytePos, fTrailing, -1, &fBytePos, &fTrailing);
                repaint();
            }
            return true;
        }
        case DGL::kKeyRight: {
            if (layout && fBytePos + fTrailing < (int)fText.size()) {
                pango_layout_move_cursor_visually(layout, true, fBytePos, fTrailing, +1, &fBytePos, &fTrailing);
                repaint();
            }
            return true;
        }
        case DGL::kKeyHome: {
            if (fBytePos != 0 || fTrailing != 0) {
                fBytePos = 0;
                fTrailing = 0;
                repaint();
            }
            return true;
        }
        case DGL::kKeyEnd: {
            if (fBytePos != 0 || fTrailing != (int)fText.size()) {
                fBytePos = (int)fText.size();
                fTrailing = 0;
                repaint();
            }
            return true;
        }
        default:
            break;
        }
    }

    return false;
}

bool TextEdit::onMouse(const MouseEvent &event)
{
    DGL::Size<uint> wsize = getSize();
    DGL::Point<int> mpos = event.pos;

    if (event.press) {
        bool inside =
            mpos.getX() >= 0 && mpos.getY() >= 0 &&
            (unsigned)mpos.getX() < wsize.getWidth() && (unsigned)mpos.getY() < wsize.getHeight();

        setHasKeyFocus(inside);
    }
    else
        setHasKeyFocus(false);

    return false;
}
