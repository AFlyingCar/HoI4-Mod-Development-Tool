
#include "ProvincePreviewDrawingArea.h"

#include "cairomm/context.h"
#include "gdkmm/general.h"

MapNormalizer::GUI::ProvincePreviewDrawingArea::ProvincePreviewDrawingArea():
    m_data(),
    m_width(0),
    m_height(0),
    m_scalex(1),
    m_scaley(1)
{ }

void MapNormalizer::GUI::ProvincePreviewDrawingArea::setScale(double x, double y)
{
    m_scalex = x;
    m_scaley = y;
}

void MapNormalizer::GUI::ProvincePreviewDrawingArea::setData(DataPtr data,
                                                             uint32_t width,
                                                             uint32_t height)
{
    m_data = data;
    m_width = width;
    m_height = height;

    queue_draw();
}

bool MapNormalizer::GUI::ProvincePreviewDrawingArea::isValid() const {
    return m_data.expired();
}

bool MapNormalizer::GUI::ProvincePreviewDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    if(auto data = m_data.lock(); data) {
        set_size_request(m_width, m_height);

        // Note: The version of Gtk being used apparently doesn't support creating
        //  Pixbuf's from data with Alpha values.
        // Because of this, we will be using Cairo directly, and bypassing
        //  Gtk/Gdk completely
        auto stride = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, m_width);

        auto cairo_image = Cairo::ImageSurface::create(const_cast<unsigned char*>(data.get()),
                                                       Cairo::FORMAT_ARGB32,
                                                       m_width, m_height,
                                                       stride);

        cr->set_source(cairo_image, 0, 0);

        // Only scale if we have a non-zero scale
        if(m_scalex != 0 && m_scaley != 0) {
            cr->scale(m_scalex, m_scaley);
        }

        cr->paint();

        return true;
    }

    return false;
}

