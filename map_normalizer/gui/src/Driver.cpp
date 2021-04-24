
#include "Driver.h"

#include "gtkmm.h"

#include "MapNormalizerApplication.h"
#include "MainWindow.h"

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

