
#include "ProvinceMapBuilder.h"

#include "UniqueColorGenerator.h"
#include "Constants.h"
#include "Util.h"

std::pair<MapNormalizer::ProvinceType, std::uint32_t>
    MapNormalizer::getProvinceType(const Color& color)
{
    auto value = colorToRGB(color);

    if(value & PROV_LAND_MASK) {
        return { ProvinceType::LAND, PROV_LAND_MASK };
    } else if(value & PROV_LAKE_MASK) {
        return { ProvinceType::LAND, PROV_LAKE_MASK };
    } else if(value & PROV_SEA_MASK) {
        return { ProvinceType::SEA, PROV_SEA_MASK };
    } else {
        return { ProvinceType::UNKNOWN, 0 };
    }
}

/*
 *
 *   4 bits       1 bit        3 bits
 * +---------+------------+--------------+
 * | Terrain | Is Coastal | Continent ID |
 * +---------+------------+--------------+
 *
 */

bool MapNormalizer::isCoastal(const Color& color, std::uint32_t mask) {
    auto value = ((color.r << 16) & (color.g << 8) & color.b) & mask;

    return ((value >> indexOfLSB(mask)) & IS_COASTAL_MASK) == 1;
}

MapNormalizer::Terrain MapNormalizer::getTerrainType(const Color& color,
                                                     std::uint32_t mask)
{
    auto value = ((color.r << 16) & (color.g << 8) & color.b) & mask;

    return static_cast<Terrain>((value >> indexOfLSB(mask)) & TERRAIN_MASK);
}

size_t MapNormalizer::getContinent(const Color& color, std::uint32_t mask) {
    auto value = ((color.r << 16) & (color.g << 8) & color.b) & mask;

    return (value >> indexOfLSB(mask)) & CONTINENT_ID_MASK;
}

MapNormalizer::ProvinceList MapNormalizer::createProvinceList(const PolygonList& shape_list)
{
    ProvinceList provinces;

    for(size_t i = 0; i < shape_list.size(); ++i) {
        auto&& shape = shape_list[i];

        auto [prov_type, mask] = getProvinceType(shape.color);

        auto is_coastal = isCoastal(shape.color, mask);
        auto terrain_type = getTerrainType(shape.color, mask);
        auto continent = getContinent(shape.color, mask);

        provinces.push_back(Province{
            i, shape.unique_color, prov_type, is_coastal, terrain_type, continent
        });
    }

    return provinces;
}

std::ostream& operator<<(std::ostream& stream,
                         const MapNormalizer::Province& province)
{
    stream << province.id << ';' << static_cast<int>(province.unique_color.r)
           << ';' << static_cast<int>(province.unique_color.g) << ';'
           << static_cast<int>(province.unique_color.b) << ';'
           << province.type << ';' << (province.coastal ? "true" : "false")
           << ';' << province.terrain << ';' << province.continent;

    return stream;
}

std::ostream& operator<<(std::ostream& stream, MapNormalizer::Terrain terrain) {
    switch(terrain) {
        case MapNormalizer::Terrain::DESERT:   stream << "desert"; break;
        case MapNormalizer::Terrain::FOREST:   stream << "forest"; break;
        case MapNormalizer::Terrain::HILLS:    stream << "hills"; break;
        case MapNormalizer::Terrain::JUNGLE:   stream << "jungle"; break;
        case MapNormalizer::Terrain::MARSH:    stream << "marsh"; break;
        case MapNormalizer::Terrain::MOUNTAIN: stream << "mountain"; break;
        case MapNormalizer::Terrain::URBAN:    stream << "urban"; break;
        case MapNormalizer::Terrain::OCEAN:    stream << "ocean"; break;
        case MapNormalizer::Terrain::LAKE:     stream << "lake"; break;
        case MapNormalizer::Terrain::PLAINS:
        default:
            stream << "plains"; break;
    }

    return stream;
}

std::ostream& operator<<(std::ostream& stream,
                         MapNormalizer::ProvinceType province)
{
    switch(province) {
        case MapNormalizer::ProvinceType::SEA:  stream << "sea"; break;
        case MapNormalizer::ProvinceType::LAKE: stream << "lake"; break;
        case MapNormalizer::ProvinceType::LAND:
        default:
            stream << "land"; break;
    }

    return stream;
}

