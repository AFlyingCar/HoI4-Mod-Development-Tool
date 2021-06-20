
#include "Driver.h"

#include "MapNormalizerApplication.h"

MapNormalizer::GUI::Driver& MapNormalizer::GUI::Driver::getInstance() {
    static Driver instance;

    return instance;
}

MapNormalizer::GUI::Driver::Driver(): m_app(), m_project(nullptr)
{ }

MapNormalizer::GUI::Driver::~Driver() {
}

bool MapNormalizer::GUI::Driver::initialize() {
#ifdef _WIN32
    g_setenv("GTK_THEME", "win32", FALSE);
#endif

    m_app = Glib::RefPtr<MapNormalizerApplication>(new MapNormalizerApplication());

    return true;
}

void MapNormalizer::GUI::Driver::run() {
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

