/**
 * @file GraphicalDebugger.cpp
 *
 * @brief File for managing the graphical debugger window of the program.
 */

#include "GraphicalDebugger.h"

// Define this to prevent the #define-ing of the min() and max() functions.
#ifndef NOMINMAX
# define NOMINMAX
#endif

#include <cmath>
#include <algorithm>

#include "BitMap.h"
#include "Options.h"
#include "Logger.h"
#include "Util.h"

HMDT::GraphicsWorker& HMDT::GraphicsWorker::getInstance() {
    static GraphicsWorker instance;

    return instance;
}

/*
 * @brief Initializes the GraphicsWorker
 *
 * @param image The image being debugged
 * @param debug_data The pixel data to display
 */
void HMDT::GraphicsWorker::init(std::shared_ptr<const MapData> map_data)
{
    m_map_data = map_data;

    auto [iwidth, iheight] = map_data->getDimensions();

    m_debug_data.reset(new unsigned char[iwidth * iheight * 3]);
}

void HMDT::GraphicsWorker::writeDebugColor(uint32_t x, uint32_t y,
                                                    const Color& c)
{
    if(m_debug_data != nullptr) {
        uint32_t w = m_map_data->getWidth();

        uint32_t index = xyToIndex(w * 3, x * 3, y);

        // Make sure we swap B and R (because BMP format sucks)
        m_debug_data[index] = c.b;
        m_debug_data[index + 1] = c.g;
        m_debug_data[index + 2] = c.r;
    }
}

void HMDT::GraphicsWorker::resetDebugData() {
    if(m_debug_data != nullptr) {
        auto data_size = m_map_data->getWidth() * m_map_data->getHeight() * 3;

        auto prov_ptr = m_map_data->getProvinces().lock();
        std::copy(prov_ptr.get(), prov_ptr.get() + data_size, m_debug_data.get());
    }
}

void HMDT::GraphicsWorker::resetDebugDataAt(const Point2D& point) {
    if(m_debug_data != nullptr) {
        uint32_t index = xyToIndex(m_map_data->getWidth(), point.x, point.y);

        m_debug_data[index] = m_map_data->getProvinces().lock()[index];
    }
}

auto HMDT::GraphicsWorker::getMapData() const
    -> std::shared_ptr<const MapData>
{
    return m_map_data;
}

void HMDT::GraphicsWorker::updateCallback(const Rectangle& rectangle) {
    m_write_callback(rectangle);
}


auto HMDT::GraphicsWorker::getWriteCallback() const
    -> const UpdateCallback&
{
    return m_write_callback;
}

void HMDT::GraphicsWorker::setWriteCallback(const std::function<void(const Rectangle&)>& callback)
{
    m_write_callback = callback;
}

void HMDT::GraphicsWorker::resetWriteCallback() {
    m_write_callback = [](const Rectangle&) { };
}

void HMDT::writeDebugColor(uint32_t x, uint32_t y, Color c) {
    GraphicsWorker& worker = GraphicsWorker::getInstance();

    worker.writeDebugColor(x, y, c);
}

