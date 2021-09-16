
#include "Driver.h"

#include "MapNormalizerApplication.h"
#include "Util.h"
#include "Logger.h"

MapNormalizer::GUI::Driver& MapNormalizer::GUI::Driver::getInstance() {
    static Driver instance;

    return instance;
}

MapNormalizer::GUI::Driver::Driver(): m_app(), m_project(nullptr)
{ }

MapNormalizer::GUI::Driver::~Driver() {
}

bool MapNormalizer::GUI::Driver::initialize() {
    WRITE_INFO("Driver initializing.");

    m_app = Glib::RefPtr<MapNormalizerApplication>(new MapNormalizerApplication());

    try {
        auto resource_path = getExecutablePath() / MN_GLIB_RESOURCES;

        WRITE_DEBUG("Loading resources from ", resource_path, "...");
        m_resources = Gio::Resource::create_from_file(resource_path.generic_string());
        WRITE_DEBUG("Done.");
    } catch(const Gio::ResourceError& e) {
        WRITE_ERROR(e.what());
        throw;
    }

    return true;
}

void MapNormalizer::GUI::Driver::run() {
    WRITE_INFO("Beginning application.");
    m_app->run();
}

/**
 * @brief Gets the main project object, if one has been loaded
 *
 * @return The main project object, or std::nullopt if one has not been loaded.
 */
auto MapNormalizer::GUI::Driver::getProject() const
    -> OptionalReference<const HProject>
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
auto MapNormalizer::GUI::Driver::getProject() 
    -> OptionalReference<HProject>
{
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
void MapNormalizer::GUI::Driver::setProject(UniqueProject&& project) {
    m_project = std::move(project);
}

/**
 * @brief Unloads the main project object.
 */
void MapNormalizer::GUI::Driver::setProject() {
    m_project = nullptr;
}

/**
 * @brief Gets the project's resources
 *
 * @return The project's resources
 */
const Glib::RefPtr<Gio::Resource> MapNormalizer::GUI::Driver::getResources() const
{
    return m_resources;
}

