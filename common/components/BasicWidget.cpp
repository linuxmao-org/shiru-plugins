#include "BasicWidget.hpp"
#include "ColorPalette.hpp"

 BasicWidget::BasicWidget(Widget *group)
     : Widget(group),
       fColorPalette(&ColorPalette::getDefault())
{
}
