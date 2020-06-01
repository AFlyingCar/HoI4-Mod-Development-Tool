#ifndef PROVINCE_MAP_BUILDER
#define PROVINCE_MAP_BUILDER

#include "Types.h"


/*
 * Province data is a 24-bit color value
 *
 *   6 bits       1 bit        3 bits      2 bits   12 bits
 * +---------+------------+--------------+--------+----------+
 * | Terrain | Is Coastal | Continent ID |  Type  | State ID |
 * +---------+------------+--------------+--------+----------+
 *
 */

namespace MapNormalizer {
    /**
     * @brief Masking values to get province data
     */
    enum ProvinceMasks: std::uint32_t {
        // Max of 64 types of terrain
        PROV_TERRAIN_MASK = 0xFC0000,

        // Boolean value
        PROV_COASTAL_MASK = 0x20000,

        // Max of 8 different continents
        PROV_CONTINENT_ID_MASK = 0x1C000,

        // Mask for getting what type of province this is
        // Valid values are UNKNOWN (0), LAND (1), LAKE (2), SEA (3)
        PROV_TYPE_MASK = 0x3000,

        // Max of 4096 different provinces
        PROV_STATE_ID_MASK = 0xFFF
    };

    ProvinceList createProvinceList(const PolygonList&);
    ProvinceType getProvinceType(std::uint32_t);
    ProvinceType getProvinceType(const Color&);
    bool isCoastal(const Color&);
    Terrain getTerrainType(const Color&);
    Continent getContinent(const Color&);
    StateID getState(const Color&);
}

#include <fstream>

std::ostream& operator<<(std::ostream&, const MapNormalizer::Province&);
std::ostream& operator<<(std::ostream&, MapNormalizer::Terrain);

#endif

