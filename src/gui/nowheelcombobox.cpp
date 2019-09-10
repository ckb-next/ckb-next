#include "nowheelcombobox.h"

void NoWheelComboBox::wheelEvent(QWheelEvent* e)
{
    // Only pass the event through if the widget is focused
    if(hasFocus())
        QComboBox::wheelEvent(e);
}
