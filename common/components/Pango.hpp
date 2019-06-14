#pragma once
#include <pango/pangocairo.h>
#include <memory>

struct GObject_delete { void operator()(gpointer p) const noexcept { g_object_unref(p); } };

typedef std::unique_ptr<PangoContext, GObject_delete> PangoContext_u;
typedef std::unique_ptr<PangoLayout, GObject_delete> PangoLayout_u;
typedef std::unique_ptr<PangoFontDescription, GObject_delete> PangoFontDescription_u;
