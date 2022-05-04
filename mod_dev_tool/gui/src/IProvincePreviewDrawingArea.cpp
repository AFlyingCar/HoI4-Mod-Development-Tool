
#include "IProvincePreviewDrawingArea.h"

HMDT::GUI::IProvincePreviewDrawingAreaBase::IProvincePreviewDrawingAreaBase():
    m_scalex(1),
    m_scaley(1),
    m_width(0),
    m_height(0)
{ }

void HMDT::GUI::IProvincePreviewDrawingAreaBase::setScale(double x,
                                                                   double y)
{
    m_scalex = x;
    m_scaley = y;
}

std::pair<double, double> HMDT::GUI::IProvincePreviewDrawingAreaBase::getScale() const
{
    return std::make_pair(m_scalex, m_scaley);
}

void HMDT::GUI::IProvincePreviewDrawingAreaBase::setDimensions(uint32_t width,
                                                                        uint32_t height)
{
    m_width = width;
    m_height = height;
}

std::pair<uint32_t, uint32_t> HMDT::GUI::IProvincePreviewDrawingAreaBase::getDimensions() const
{
    return std::make_pair(m_width, m_height);
}

/**
 * @brief Calculates the correct scaling for the preview so that it fits in the
 *        drawing area without being stretched too much
 */
void HMDT::GUI::IProvincePreviewDrawingAreaBase::calcScale() {
    auto* parent = getParent();

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


