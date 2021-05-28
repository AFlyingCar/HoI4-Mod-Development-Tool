
#include "ProvincePreviewDrawingArea.h"

#include "cairomm/context.h"
#include "gdkmm/general.h"
#include "gtkmm/container.h"

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

/**
 * @brief Calculates the correct scaling for the preview so that it fits in the
 *        drawing area without being stretched too much
 */
void MapNormalizer::GUI::ProvincePreviewDrawingArea::calcScale() {
    auto* parent = get_parent();

    if(parent != nullptr) {
        double pwidth = parent->get_width();
        double pheight = parent->get_height();

        // Scale to the smallest dimension of the parent window
        // But only scale down if the image is too large, don't worry about
        //  trying to scale up a smaller image
        if(m_height > 512) {
            m_scaley = (pheight / m_height);
            m_scalex = (pheight / m_height);
        } else if(pwidth < m_width) {
            m_scalex = (pwidth / m_width);
            m_scaley = (pwidth / m_width);
        } else {
            m_scalex = 1.0;
            m_scaley = 1.0;
        }
    }
}

void MapNormalizer::GUI::ProvincePreviewDrawingArea::setData(DataPtr data,
                                                             uint32_t width,
                                                             uint32_t height)
{
    m_data = data;
    m_width = width;
    m_height = height;

    calcScale();

    queue_draw();
}

bool MapNormalizer::GUI::ProvincePreviewDrawingArea::isValid() const {
    return m_data.expired();
}

bool MapNormalizer::GUI::ProvincePreviewDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    if(auto data = m_data.lock(); data) {
        auto [width, height] = std::make_pair(m_width * m_scalex,
                                              m_height * m_scaley);

        set_size_request(width, height);

        // Note: The version of Gtk being used apparently doesn't support creating
        //  Pixbuf's from data with Alpha values.
        // Because of this, we will be using Cairo directly, and bypassing
        //  Gtk/Gdk completely
        auto stride = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, m_width);

        auto cairo_image = Cairo::ImageSurface::create(const_cast<unsigned char*>(data.get()),
                                                       Cairo::FORMAT_ARGB32,
                                                       m_width, m_height,
                                                       stride);

        // Only scale if we have a non-zero scale
        if(m_scalex != 0 && m_scaley != 0) {
            cr->scale(m_scalex, m_scaley);
        }

        cr->set_source(cairo_image, 0, 0);

        cr->paint();

        return true;
    }

    return false;
}

