
#include "Window.h"

#include "Logger.h"

MapNormalizer::GUI::Window::Window(const std::string& window_name,
                                   Gtk::Application& application):
    m_window_name(window_name),
    m_application(application)
{
    set_title(window_name);
}

MapNormalizer::GUI::Window::~Window() {
    delete m_box;
}

bool MapNormalizer::GUI::Window::initialize() {
    WRITE_DEBUG("Window::initialize");

    // Make sure the window is maximized first
    maximize();
    set_show_menubar(true);

    if(!initializeActions()) {
        return false;
    }

    add(*(m_box = new Gtk::Box(getDisplayOrientation())));

    if(!initializeWidgets()) {
        return false;
    }

    show_all_children();

    WRITE_DEBUG("done");

    return true;
}

Gtk::Box* MapNormalizer::GUI::Window::getBox() {
    return m_box;
}

Gtk::Orientation MapNormalizer::GUI::Window::getDisplayOrientation() const {
    return Gtk::Orientation::ORIENTATION_HORIZONTAL;
}

Gtk::Application* MapNormalizer::GUI::Window::getApplication() {
    return &m_application;
}

void MapNormalizer::GUI::Window::addWidgetToParent(Gtk::Widget& widget) {
    if(m_box != nullptr) {
        m_box->add(widget);
    }
}

