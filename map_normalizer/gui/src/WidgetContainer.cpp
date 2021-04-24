
#include "WidgetContainer.h"

MapNormalizer::GUI::WidgetContainer::~WidgetContainer() {
    for(auto&& widget : m_widgets) {
        delete widget;
    }
}

