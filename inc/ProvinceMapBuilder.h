#ifndef PROVINCE_MAP_BUILDER
#define PROVINCE_MAP_BUILDER

#include "Types.h"

namespace MapNormalizer {
    ProvinceList createProvinceList(const PolygonList&);
    ProvinceType getProvinceType(const Color&);
    bool isCoastal(const Polygon&);
    Terrain getTerrainType(const Color&);
    size_t getContinent(const Color&);
}

#include <fstream>

std::ostream& operator<<(std::ostream&, const MapNormalizer::Province&);
std::ostream& operator<<(std::ostream&, MapNormalizer::Terrain);
std::ostream& operator<<(std::ostream&, MapNormalizer::ProvinceType);

#endif

