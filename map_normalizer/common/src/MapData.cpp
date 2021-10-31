#include "MapData.h"

#include "Util.h"

MapNormalizer::MapData::MapData(uint32_t width, uint32_t height):
    m_width(width),
    m_height(height),
    m_input(new uint8_t[width * height * 3]{ 0 }),
    m_provinces(new uint8_t[width * height * 3]{ 0 }),
    m_province_outlines(new uint8_t[width * height * 4]{ 0 }),
    m_closed(false)
{
}

void MapNormalizer::MapData::close() {
    m_closed = true;
}

uint32_t MapNormalizer::MapData::getWidth() const {
    return m_width;
}

uint32_t MapNormalizer::MapData::getHeight() const {
    return m_height;
}

std::pair<uint32_t, uint32_t> MapNormalizer::MapData::getDimensions() const {
    return std::make_pair(m_width, m_height);
}

bool MapNormalizer::MapData::isClosed() const {
    return m_closed;
}

///////////////////////////////////////////////////////////////////////////////

auto MapNormalizer::MapData::getInput() -> MapType {
    return m_input;
}

auto MapNormalizer::MapData::getInput() const -> ConstMapType {
    return m_input;
}

auto MapNormalizer::MapData::getProvinces() -> MapType {
    return m_provinces;
}

auto MapNormalizer::MapData::getProvinces() const -> ConstMapType {
    return m_provinces;
}

auto MapNormalizer::MapData::getProvinceOutlines() -> MapType {
    return m_province_outlines;
}

auto MapNormalizer::MapData::getProvinceOutlines() const -> ConstMapType {
    return m_province_outlines;
}

