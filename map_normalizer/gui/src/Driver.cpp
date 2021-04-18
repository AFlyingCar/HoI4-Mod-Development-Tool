
#include "Driver.h"

MapNormalizer::GUI::Driver::Driver(): m_platform(nullptr),
                                      m_gui(nullptr),
                                      m_image_loader()
{
}

MapNormalizer::GUI::Driver::~Driver() {
    if(m_gui != nullptr) {
        m_platform->shutdown();
        m_gui->shutdown();
    }
}

void MapNormalizer::GUI::Driver::initialize() {
    // Do nothing if we are already initialized
    if(m_gui != nullptr) return;

    m_platform.reset(new MyGUI::OpenGLPlatform());
    m_platform->initialise(&m_image_loader);

    m_gui.reset(new MyGUI::Gui());
    m_gui->initialise();
}

