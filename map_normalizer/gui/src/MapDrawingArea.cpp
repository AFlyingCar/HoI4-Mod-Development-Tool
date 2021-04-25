
#include "MapDrawingArea.h"

#include "cairomm/context.h"
#include "gdkmm/general.h"

#include "Logger.h"
#include "Constants.h"

#include "GraphicalDebugger.h"

MapNormalizer::GUI::MapDrawingArea::MapDrawingArea()
{
    // Mark that we want to receive button presses
    add_events(Gdk::BUTTON_PRESS_MASK);
}

bool MapNormalizer::GUI::MapDrawingArea::hasData() const {
    return m_graphics_data_ptr != nullptr && *m_graphics_data_ptr != nullptr &&
           m_image_ptr != nullptr && *m_image_ptr != nullptr;
}

bool MapNormalizer::GUI::MapDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Do nothing if we have no graphics data to actually render
    if(!hasData()) {
        return true;
    }

    auto iwidth = (*m_image_ptr)->info_header.width;
    auto iheight = (*m_image_ptr)->info_header.height;

    set_size_request(iwidth, iheight);

    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create_from_data(*m_graphics_data_ptr, Gdk::Colorspace::COLORSPACE_RGB, false, 8, iwidth, iheight, iwidth * 3);

    // Draw the image in the center of the drawing area (or the middle part of
    //  the imagee if it is larger than the drawing area)
    // TODO: We should have a way to move the image around/zoom
    //  Also, the specific image should not be affecting the size of the window itself
    Gdk::Cairo::set_source_pixbuf(cr, image, 0, 0);

    // TODO: cr has scale+translate methods, we should use those
    cr->scale(1, 1);

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
        auto x = event->x;
        auto y = event->y;

        if(event->state & GDK_SHIFT_MASK) {
            writeDebug("Got shift+click!");
            // TODO: Mark which shape we are actually selecting rather than just
            //  drawing a pixel at the point where we click
            writeDebugColor(x, y, Color{0, 255, 255});
            queue_draw_area(x, y, 1, 1);
        } else {
            // TODO: Remove this
            writeDebug("Got mouse button at (", x, ',', y, ")!");

            // TODO: Mark which shape we are actually selecting rather than just
            //  drawing a pixel at the point where we click
            writeDebugColor(x, y, CURSOR_COLOR);
            queue_draw_area(x, y, 1, 1);
        }
    }

    return true;
}

void MapNormalizer::GUI::MapDrawingArea::setGraphicsDataPtr(unsigned char* const* data_ptr)
{
    m_graphics_data_ptr = data_ptr;
}

void MapNormalizer::GUI::MapDrawingArea::setImagePtr(BitMap* const* image_ptr)
{
    m_image_ptr = image_ptr;
}

void MapNormalizer::GUI::MapDrawingArea::graphicsUpdateCallback(const Rectangle& rectangle)
{
    if(rectangle.w == 0 && rectangle.h == 0) {
        return;
    }

    queue_draw_area(rectangle.x, rectangle.y, rectangle.w, rectangle.h);
}

