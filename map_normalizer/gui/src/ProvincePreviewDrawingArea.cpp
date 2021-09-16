
#include "ProvincePreviewDrawingArea.h"

#include "cairomm/context.h"
#include "gdkmm/general.h"
#include "gtkmm/container.h"

MapNormalizer::GUI::ProvincePreviewDrawingArea::ProvincePreviewDrawingArea():
    m_data()
{ }

void MapNormalizer::GUI::ProvincePreviewDrawingArea::setData(DataPtr data,
                                                             uint32_t width,
                                                             uint32_t height)
{
    m_data = data;

    setDimensions(width, height);

    calcScale();

    queue_draw();
}

bool MapNormalizer::GUI::ProvincePreviewDrawingArea::isValid() const {
    return m_data.expired();
}

bool MapNormalizer::GUI::ProvincePreviewDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    if(auto data = m_data.lock(); data) {
        auto [scalex, scaley] = getScale();
        auto [width, height] = getDimensions();

        auto [scaled_width, scaled_height] = std::make_pair(width * scalex,
                                                            height * scaley);

        set_size_request(scaled_width, scaled_height);

        // Note: The version of Gtk being used apparently doesn't support creating
        //  Pixbuf's from data with Alpha values.
        // Because of this, we will be using Cairo directly, and bypassing
        //  Gtk/Gdk completely
        auto stride = Cairo::ImageSurface::format_stride_for_width(Cairo::FORMAT_ARGB32, width);

        auto cairo_image = Cairo::ImageSurface::create(const_cast<unsigned char*>(data.get()),
                                                       Cairo::FORMAT_ARGB32,
                                                       width, height,
                                                       stride);

        // Only scale if we have a non-zero scale
        if(scalex != 0 && scaley != 0) {
            cr->scale(scalex, scaley);
        }

        cr->set_source(cairo_image, 0, 0);

        cr->paint();

        return true;
    }

    return false;
}

Gtk::Widget* MapNormalizer::GUI::ProvincePreviewDrawingArea::getParent() {
    return get_parent();
}

