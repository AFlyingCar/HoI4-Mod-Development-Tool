
#include "MapDrawingArea.h"

#include "cairomm/context.h"
#include "gdkmm/general.h"
#include "gdk/gdkcairo.h"
#include "gtkmm/container.h"

#include "Logger.h"
#include "Constants.h"

#include "GraphicalDebugger.h"
#include "Util.h"

MapNormalizer::GUI::MapDrawingArea::MapDrawingArea():
    m_graphics_data(nullptr),
    m_image(nullptr),
    m_on_select([](auto...) { }),      // The default callback does nothing
    m_on_multiselect([](auto...) { }),
    m_scale_factor(DEFAULT_ZOOM)
{
    // Mark that we want to receive button presses
    add_events(Gdk::BUTTON_PRESS_MASK);
}

bool MapNormalizer::GUI::MapDrawingArea::hasData() const {
    return m_graphics_data != nullptr && m_image != nullptr;
}

bool MapNormalizer::GUI::MapDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Do nothing if we have no graphics data to actually render
    if(!hasData()) {
        return true;
    }

    // If a province is selected, then go ahead and draw the province preview
    //  on top of the map again
    if(m_selection) {
        auto&& [width, height] = calcDims(m_selection->bounding_box);

        auto stride = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, width);
        auto province_image = Cairo::ImageSurface::create(m_selection->data.get(),
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
        full_cr->set_source(province_image, m_selection->bounding_box.bottom_left.x, m_selection->bounding_box.top_right.y);
#else
        auto posx = m_selection->bounding_box.bottom_left.x;
        auto posy = m_selection->bounding_box.top_right.y;

        full_cr->scale(m_scale_factor, m_scale_factor);
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

bool MapNormalizer::GUI::MapDrawingArea::on_button_press_event(GdkEventButton* event)
{
    if(!hasData()) {
        return true;
    }

    // Is it a left-click?
    if(event->type == GDK_BUTTON_PRESS && event->button == 1) {
        // Note that x and y will be the values after scaling. If we want the
        //  true coordinates, we have to invert the scaling
        auto x = event->x * (1 / m_scale_factor);
        auto y = event->y * (1 / m_scale_factor);

        if(event->state & GDK_SHIFT_MASK) {
            m_on_multiselect(x, y);
        } else {
            m_on_select(x, y);
        }
    }

    return true;
}

void MapNormalizer::GUI::MapDrawingArea::setGraphicsData(const unsigned char* data)
{
    m_graphics_data = data;
}

void MapNormalizer::GUI::MapDrawingArea::setImage(const BitMap* image) {
    m_image = image;
}

void MapNormalizer::GUI::MapDrawingArea::setData(const BitMap* image,
                                                 const unsigned char* data)
{
    setImage(image);
    setGraphicsData(data);

    resetZoom();
}

void MapNormalizer::GUI::MapDrawingArea::graphicsUpdateCallback(const Rectangle& rectangle)
{
    if(rectangle.w == 0 && rectangle.h == 0) {
        return;
    }

    queue_draw_area(rectangle.x, rectangle.y, rectangle.w, rectangle.h);
}

void MapNormalizer::GUI::MapDrawingArea::setOnProvinceSelectCallback(const SelectionCallback& callback)
{
    m_on_select = callback;
}

void MapNormalizer::GUI::MapDrawingArea::setOnMultiProvinceSelectionCallback(const SelectionCallback& callback)
{
    m_on_multiselect = callback;
}

void MapNormalizer::GUI::MapDrawingArea::setSelection() {
    m_selection = std::nullopt;
}

void MapNormalizer::GUI::MapDrawingArea::setSelection(const SelectionInfo& selection)
{
    m_selection = selection;
}

void MapNormalizer::GUI::MapDrawingArea::zoom(ZoomDirection direction) {
    switch(direction) {
        case ZoomDirection::IN:
            zoom(ZOOM_FACTOR);
            break;
        case ZoomDirection::OUT:
            zoom(-ZOOM_FACTOR);
            break;
        case ZoomDirection::RESET:
            resetZoom();
            break;
    }
}

void MapNormalizer::GUI::MapDrawingArea::zoom(double scale_factor_delta) {
    m_scale_factor += scale_factor_delta;

    rebuildImageCache();

    // We need to redraw the entire map if we zoom in/out
    queue_draw();
}

/**
 * @brief Will reset the zoom level
 */
void MapNormalizer::GUI::MapDrawingArea::resetZoom() {
    // Do nothing if there is no data loaded
    if(!hasData()) return;

    auto* parent = get_parent();

    if(parent == nullptr) {
        WRITE_WARN("MapDrawingArea has no parent, setting zoom to ", DEFAULT_ZOOM);

        m_scale_factor = DEFAULT_ZOOM;
    } else {
        double pwidth = parent->get_width();
        double pheight = parent->get_height();

        double iheight = m_image->info_header.height;

        // Scale to the smallest dimension of the parent window
        // But only scale down if the image is too large, don't worry about
        //  trying to scale up a smaller image
        if(pheight <= pwidth && pheight < iheight) {
            m_scale_factor = (pheight / iheight);
        } else {
            m_scale_factor = DEFAULT_ZOOM;
        }
    }

    WRITE_DEBUG("Reset zoom to ", m_scale_factor);

    // Rebuild the cached image
    rebuildImageCache();
    queue_draw();
}

/**
 * @brief Scales the cached Pixbuf up or down based on the current scale factor
 */
void MapNormalizer::GUI::MapDrawingArea::rebuildImageCache() {
    if(hasData()) {
        auto iwidth = m_image->info_header.width;
        auto iheight = m_image->info_header.height;

        auto siwidth = iwidth * m_scale_factor;
        auto siheight = iheight * m_scale_factor;

        m_image_pixbuf = Gdk::Pixbuf::create_from_data(m_graphics_data, Gdk::Colorspace::COLORSPACE_RGB, false, 8, iwidth, iheight, iwidth * 3);

        m_image_pixbuf = m_image_pixbuf->scale_simple(siwidth, siheight, Gdk::INTERP_BILINEAR);

        set_size_request(siwidth, siheight);
    } else {
        m_image_pixbuf.reset();
    }
}

