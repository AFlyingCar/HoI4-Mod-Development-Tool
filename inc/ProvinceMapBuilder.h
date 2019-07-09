#ifndef PROVINCE_MAP_BUILDER
#define PROVINCE_MAP_BUILDER

#include "Types.h"

namespace MapNormalizer {
    const auto PROV_LAND_MASK = 0xFF0000;
    const auto PROV_LAKE_MASK = 0x00FF00;
    const auto PROV_SEA_MASK  = 0x0000FF;

    const auto TERRAIN_MASK      = 0b11110000;
    const auto IS_COASTAL_MASK   = 0b00001000;
    const auto CONTINENT_ID_MASK = 0x00000111; // Maximum of 8 total continents

    ProvinceList createProvinceList(const PolygonList&);
    std::pair<ProvinceType, std::uint32_t> getProvinceType(std::uint32_t);
    std::pair<ProvinceType, std::uint32_t> getProvinceType(const Color&);
    bool isCoastal(const Color&, std::uint32_t);
    Terrain getTerrainType(const Color&, std::uint32_t);
    size_t getContinent(const Color&, std::uint32_t);
}

#include <fstream>

std::ostream& operator<<(std::ostream&, const MapNormalizer::Province&);
std::ostream& operator<<(std::ostream&, MapNormalizer::Terrain);

#endif

