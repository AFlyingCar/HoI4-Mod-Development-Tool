/**
 * @file GraphicalDebugger.cpp
 *
 * @brief File for managing the graphical debugger window of the program.
 */

#include "GraphicalDebugger.h"

// Define this to prevent the #define-ing of the min() and max() functions.
#define NOMINMAX
#include <cmath>
#include <algorithm>

#include "BitMap.h"
#include "Options.h"
#include "Logger.h"
#include "Util.h"

MapNormalizer::GraphicsWorker& MapNormalizer::GraphicsWorker::getInstance() {
    static GraphicsWorker instance;

    return instance;
}

/*
 * @brief Initializes the GraphicsWorker
 *
 * @param image The image being debugged
 * @param debug_data The pixel data to display
 */
void MapNormalizer::GraphicsWorker::init(const BitMap* image,
                                         unsigned char* debug_data)
{
    // Make sure we delete the old data first if we have any
    if(m_debug_data != nullptr) {
        delete[] m_debug_data;
    }

    if(m_image != nullptr) {
        delete m_image;
    }

    m_image = image;
    m_debug_data = debug_data;
}

void MapNormalizer::GraphicsWorker::writeDebugColor(uint32_t x, uint32_t y,
                                                    Color c)
{
    if(m_debug_data != nullptr) {
        uint32_t w = m_image->info_header.width;

        uint32_t index = xyToIndex(w * 3, x * 3, y);

        // Make sure we swap B and R (because BMP format sucks)
        m_debug_data[index] = c.b;
        m_debug_data[index + 1] = c.g;
        m_debug_data[index + 2] = c.r;
    }
}

void MapNormalizer::GraphicsWorker::resetDebugData() {
    if(m_debug_data != nullptr) {
        auto data_size = m_image->info_header.width * m_image->info_header.height * 3;

        std::copy(m_image->data, m_image->data + data_size, m_debug_data);
    }
}

void MapNormalizer::GraphicsWorker::resetDebugDataAt(const Point2D& point) {
    if(m_debug_data != nullptr) {
        uint32_t index = xyToIndex(m_image, point.x, point.y);

        m_debug_data[index] = m_image->data[index];
    }
}

const unsigned char* MapNormalizer::GraphicsWorker::getDebugData() const {
    return m_debug_data;
}

const MapNormalizer::BitMap* MapNormalizer::GraphicsWorker::getImage() const {
    return m_image;
}

void MapNormalizer::GraphicsWorker::updateCallback(const Rectangle& rectangle) {
    m_write_callback(rectangle);
}


auto MapNormalizer::GraphicsWorker::getWriteCallback() const
    -> const UpdateCallback&
{
    return m_write_callback;
}

void MapNormalizer::GraphicsWorker::setWriteCallback(const std::function<void(const Rectangle&)>& callback)
{
    m_write_callback = callback;
}

void MapNormalizer::GraphicsWorker::resetWriteCallback() {
    m_write_callback = [](const Rectangle&) { };
}

void MapNormalizer::writeDebugColor(uint32_t x, uint32_t y, Color c) {
    GraphicsWorker& worker = GraphicsWorker::getInstance();

    worker.writeDebugColor(x, y, c);
}

