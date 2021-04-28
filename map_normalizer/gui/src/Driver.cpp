
#include "Driver.h"

#include "MapNormalizerApplication.h"

MapNormalizer::GUI::Driver& MapNormalizer::GUI::Driver::getInstance() {
    static Driver instance;

    return instance;
}

MapNormalizer::GUI::Driver::Driver(): m_app()
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

