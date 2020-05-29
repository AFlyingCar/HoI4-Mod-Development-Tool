
#include "ProvinceMapBuilder.h"

#include "UniqueColorGenerator.h"
#include "Constants.h"
#include "Terrain.h"
#include "Util.h"

auto MapNormalizer::getProvinceType(std::uint32_t color) -> ProvinceType {
    auto value = color & PROV_TYPE_MASK;

    switch(value >> indexOfLSB(value)) {
        case 3:
            return ProvinceType::SEA;
        case 2:
            return ProvinceType::LAKE;
        case 1:
            return ProvinceType::LAND;
        case 0:
        default:
            return ProvinceType::UNKNOWN;
    }
}

auto MapNormalizer::getProvinceType(const Color& color) -> ProvinceType {
    return getProvinceType(colorToRGB(color));
}

bool MapNormalizer::isCoastal(const Color& color) {
    // Masks the value to just have coastal bit
    auto value = (((color.r << 16) & (color.g << 8) & color.b) & PROV_COASTAL_MASK);

    // Shift down until the least significant bit is the first one
    //  It should be the only bit remaining
    return (value >> indexOfLSB(value)) == 1;
}

auto MapNormalizer::getTerrainType(const Color& color) -> Terrain {
    auto value = ((color.r << 16) & (color.g << 8) & color.b) & PROV_TERRAIN_MASK;

    return static_cast<Terrain>(value >> indexOfLSB(value));
}

auto MapNormalizer::getContinent(const Color& color) -> Continent {
    auto value = ((color.r << 16) & (color.g << 8) & color.b) & PROV_CONTINENT_ID_MASK;

    return static_cast<Continent>(value >> indexOfLSB(value));
}

auto MapNormalizer::getState(const Color& color) -> State {
    auto value = ((color.r << 16) & (color.g << 8) & color.b) & PROV_STATE_ID_MASK;

    // No need for a shift, since states are already at the LSB
    return value;
}

MapNormalizer::ProvinceList MapNormalizer::createProvinceList(const PolygonList& shape_list)
{
    ProvinceList provinces;

    for(size_t i = 0; i < shape_list.size(); ++i) {
        auto&& shape = shape_list[i];

        auto prov_type = getProvinceType(shape.color);

        auto is_coastal = isCoastal(shape.color);
        auto terrain_type = getTerrainType(shape.color);
        auto continent = getContinent(shape.color);
        auto state = getState(shape.color);

        provinces.push_back(Province{
            i, shape.unique_color,
            prov_type, is_coastal, terrain_type, continent, state
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
    stream << MapNormalizer::getTerrainIdentifier(terrain);

    return stream;
}

