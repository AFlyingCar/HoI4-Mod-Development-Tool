
#include "Driver.h"

#include "Application.h"
#include "Util.h"
#include "Logger.h"

HMDT::GUI::Driver& HMDT::GUI::Driver::getInstance() {
    static Driver instance;

    return instance;
}

HMDT::GUI::Driver::Driver(): m_app(), m_project(nullptr)
{ }

HMDT::GUI::Driver::~Driver() {
}

bool HMDT::GUI::Driver::initialize() {
    WRITE_INFO("Driver initializing.");

    m_app = Glib::RefPtr<Application>(new Application());

    try {
        auto resource_path = getExecutablePath() / HMDT_GLIB_RESOURCES;

        WRITE_DEBUG("Loading resources from ", resource_path, "...");
        m_resources = Gio::Resource::create_from_file(resource_path.generic_string());
        m_resources->register_global(); // Make sure it can be accessed anywhere
        WRITE_DEBUG("Done.");
    } catch(const Gio::ResourceError& e) {
        WRITE_ERROR(e.what());
        throw;
    }

    return true;
}

void HMDT::GUI::Driver::run() {
    WRITE_INFO("Beginning application.");
    m_app->run();
}

/**
 * @brief Gets the main project object, if one has been loaded
 *
 * @return The main project object, or std::nullopt if one has not been loaded.
 */
auto HMDT::GUI::Driver::getProject() const -> OptionalReference<const HProject>
{
    if(m_project) {
        return *m_project;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Gets the main project object, if one has been loaded
 *
 * @return The main project object, or std::nullopt if one has not been loaded.
 */
auto HMDT::GUI::Driver::getProject() -> OptionalReference<HProject> {
    if(m_project) {
        return *m_project;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Sets the main project object.
 *
 * @param project 
 */
void HMDT::GUI::Driver::setProject(UniqueProject&& project) {
    m_project = std::move(project);
}

/**
 * @brief Unloads the main project object.
 */
void HMDT::GUI::Driver::setProject() {
    m_project = nullptr;
}

/**
 * @brief Gets the project's resources
 *
 * @return The project's resources
 */
const Glib::RefPtr<Gio::Resource> HMDT::GUI::Driver::getResources() const {
    return m_resources;
}

