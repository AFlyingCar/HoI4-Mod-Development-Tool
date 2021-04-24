
#include "MapDrawingArea.h"

#include "cairomm/context.h"
#include "gdkmm/general.h"

MapNormalizer::GUI::MapDrawingArea::MapDrawingArea()
{ }

#include <iostream>

bool MapNormalizer::GUI::MapDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    // Do nothing if we have no graphics data to actually render
    if(m_graphics_data_ptr == nullptr || *m_graphics_data_ptr == nullptr ||
       m_image_ptr == nullptr || *m_image_ptr == nullptr)
    {
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

void MapNormalizer::GUI::MapDrawingArea::setGraphicsDataPtr(unsigned char* const* data_ptr)
{
    m_graphics_data_ptr = data_ptr;
}

void MapNormalizer::GUI::MapDrawingArea::setImagePtr(BitMap* const* image_ptr)
{
    m_image_ptr = image_ptr;
}

