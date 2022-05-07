/**
 * @file ProvinceMapBuilder.h
 *
 * @brief Defines functions for creating a list of provinces from detected
 *        shapes.
 * @par Province data is stored as a 24-bit color value in the input .BMP.
 * @verbatim
           6 bits       1 bit        3 bits      2 bits   12 bits
         +---------+------------+--------------+--------+----------+
         | Terrain | Is Coastal | Continent ID |  Type  | State ID |
         +---------+------------+--------------+--------+----------+
   @endverbatim
 *
 */

#ifndef PROVINCE_MAP_BUILDER
# define PROVINCE_MAP_BUILDER

# include "Types.h"

namespace HMDT {
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
    [[deprecated]] bool isCoastal(const Color&);
    [[deprecated]] TerrainID getTerrainType(const Color&);
    [[deprecated]] Continent getContinent(const Color&);
    [[deprecated]] StateID getState(const Color&);

    std::ostream& operator<<(std::ostream&, const HMDT::Province&);
}

#endif

