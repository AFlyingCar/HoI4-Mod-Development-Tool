
#include "Window.h"

#include "Logger.h"
#include "StatusCodes.h"

HMDT::GUI::Window::Window(const std::string& window_name,
                          Gtk::Application& application):
    m_window_name(window_name),
    m_application(application)
{
    set_title(window_name);
}

HMDT::GUI::Window::~Window() {
    delete m_box;
}

bool HMDT::GUI::Window::initialize() {
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

    if(!initializeFinal()) {
        return false;
    }

    show_all_children();

    WRITE_DEBUG("done");

    return true;
}

auto HMDT::GUI::Window::setupDispatcher(const std::function<void()>& callback)
    -> Maybe<uint32_t>
{
    ++m_next_dispatcher_id;

    // If this ID is 0 then we have overflowed
    if(m_next_dispatcher_id == 0) {
        WRITE_WARN("m_next_dispatcher_id has overflowed! If any dispatcher has "
                   "not finished running yet then we may get conflicts.");
    }

    WRITE_DEBUG("Setting up dispatcher with id=", m_next_dispatcher_id);

    m_dispatchers.emplace(std::piecewise_construct,
                          std::forward_as_tuple(m_next_dispatcher_id),
                          std::forward_as_tuple());
    m_dispatchers[m_next_dispatcher_id].connect(callback);

    return m_next_dispatcher_id;
}

auto HMDT::GUI::Window::setupDispatcher(const std::function<void(uint32_t)>& callback)
    -> Maybe<uint32_t>
{
    auto next_id = m_next_dispatcher_id + 1;
    return setupDispatcher([next_id, callback]() {
        callback(next_id);
    });
}

HMDT::MaybeVoid HMDT::GUI::Window::notifyDispatcher(uint32_t id) {
    if(m_dispatchers.count(id) == 0) {
        RETURN_ERROR(STATUS_DISPATCHER_DOES_NOT_EXIST);
    }

    m_dispatchers[id].emit();

    return STATUS_SUCCESS;
}

HMDT::MaybeVoid HMDT::GUI::Window::teardownDispatcher(uint32_t id) {
    WRITE_DEBUG("Tearing down dispatcher with id=", id);

    if(m_dispatchers.count(id) == 0) {
        RETURN_ERROR(STATUS_DISPATCHER_DOES_NOT_EXIST);
    }

    m_dispatchers.erase(id);

    return STATUS_SUCCESS;
}

Gtk::Box* HMDT::GUI::Window::getBox() {
    return m_box;
}

Gtk::Orientation HMDT::GUI::Window::getDisplayOrientation() const {
    return Gtk::Orientation::ORIENTATION_HORIZONTAL;
}

Gtk::Application* HMDT::GUI::Window::getApplication() {
    return &m_application;
}

void HMDT::GUI::Window::addWidgetToParent(Gtk::Widget& widget) {
    if(m_box != nullptr) {
        m_box->add(widget);
    }
}

