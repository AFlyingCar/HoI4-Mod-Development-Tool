
#include "ProvinceRenderingView.h"

void MapNormalizer::GUI::GL::ProvinceRenderingView::init() {
    MapRenderingViewBase::init();
    // TODO
}

void MapNormalizer::GUI::GL::ProvinceRenderingView::render() {
    // Render the normal map first
    MapRenderingViewBase::render();

    // Now render the outlines on top of it
    
    // Then render the selected province (if there is one selected) on top of that
}
