
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
    m_app = Glib::RefPtr<MapNormalizerApplication>(new MapNormalizerApplication());

    return true;
}

void MapNormalizer::GUI::Driver::run() {
    m_app->run();
}

auto MapNormalizer::GUI::Driver::getProject() const
    -> OptionalReference<const HProject>
{
    if(m_project) {
        return *m_project;
    } else {
        return std::nullopt;
    }
}

auto MapNormalizer::GUI::Driver::getProject() 
    -> OptionalReference<HProject>
{
    if(m_project) {
        return *m_project;
    } else {
        return std::nullopt;
    }
}

void MapNormalizer::GUI::Driver::setProject(UniqueProject&& project) {
    m_project = std::move(project);
}

void MapNormalizer::GUI::Driver::setProject() {
    m_project = nullptr;
}

