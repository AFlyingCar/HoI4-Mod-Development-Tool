
#include "MapDrawingArea.h"

#include "cairomm/context.h"
#include "gdkmm/general.h"
#include "gdk/gdkcairo.h"
#include "gtkmm/container.h"

#include "Logger.h"
#include "Constants.h"

#include "GraphicalDebugger.h"
#include "Util.h"

bool HMDT::GUI::MapDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Do nothing if we have no graphics data to actually render
    if(!hasData()) {
        return true;
    }

    // If a province is selected, then go ahead and draw the province preview
    //  on top of the map again
    if(!getSelections().empty()) {
        auto&& [width, height] = calcDims(getSelections().begin()->bounding_box);

        auto stride = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, width);
        auto province_image = Cairo::ImageSurface::create(getSelections().begin()->data.get(),
                                                          Cairo::FORMAT_ARGB32,
                                                          width, height,
                                                          stride);

        // Manually create the surface from the pixbuf so we can do extra stuff
        //  with it
        Cairo::RefPtr<Cairo::Surface> full_image(new Cairo::Surface(gdk_cairo_surface_create_from_pixbuf(m_image_pixbuf->gobj(), 1, NULL)));

        auto province_cr = Cairo::Context::create(province_image);
        auto full_cr = Cairo::Context::create(full_image);

        full_cr->set_source(full_image, 0, 0);
        full_cr->paint();

        // TODO: We should probably do something fancy here to draw the province
        //  as a single color
        // See: https://www.cairographics.org/operators/
#if 0
        // Make sure the province is only a single color

        province_cr->set_source(province_image, 0, 0);
        province_cr->paint();
        province_cr->set_operator(Cairo::OPERATOR_DEST_OUT);
        province_cr->set_source_rgb(1.0, 0, 0);
        province_cr->paint();
        province_cr->set_operator(Cairo::OPERATOR_SOURCE); // Reset the operator

        // Paint the province onto the full image
        full_cr->set_source(province_image, getSelection()->bounding_box.bottom_left.x, getSelection()->bounding_box.top_right.y);
#else
        auto posx = getSelections().begin()->bounding_box.bottom_left.x;
        auto posy = getSelections().begin()->bounding_box.top_right.y;

        full_cr->scale(getScaleFactor(), getScaleFactor());
        full_cr->set_source(province_image, posx, posy);
#endif
        full_cr->paint();

        // Paint the full image
        cr->set_source(full_image, 0, 0);
    } else {
        // Draw the image in the center of the drawing area (or the middle part of
        //  the imagee if it is larger than the drawing area)
        // TODO: We should have a way to move the image around/zoom
        //  Also, the specific image should not be affecting the size of the window itself
        Gdk::Cairo::set_source_pixbuf(cr, m_image_pixbuf, 0, 0);
    }

    cr->paint();

    return true;
}

/**
 * @brief Scales the cached Pixbuf up or down based on the current scale factor
 */
void HMDT::GUI::MapDrawingArea::rebuildImageCache() {
    if(hasData()) {
        auto [iwidth, iheight] = getMapData()->getDimensions();

        auto siwidth = iwidth * getScaleFactor();
        auto siheight = iheight * getScaleFactor();

        auto prov_ptr = getMapData()->getProvinces().lock();
        m_image_pixbuf = Gdk::Pixbuf::create_from_data(prov_ptr.get(), Gdk::Colorspace::COLORSPACE_RGB, false, 8, iwidth, iheight, iwidth * 3);

        m_image_pixbuf = m_image_pixbuf->scale_simple(siwidth, siheight, Gdk::INTERP_BILINEAR);

        set_size_request(siwidth, siheight);
    } else {
        m_image_pixbuf.reset();
    }
}

void HMDT::GUI::MapDrawingArea::onZoom() {
    // Rebuild the cached image
    rebuildImageCache();

    // We need to redraw the entire map if we zoom in/out
    queue_draw();
}

void HMDT::GUI::MapDrawingArea::init() { }

