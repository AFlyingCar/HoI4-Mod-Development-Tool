
#include "ProvinceMapBuilder.h"

#include "UniqueColorGenerator.h"
#include "Constants.h"
#include "Terrain.h"
#include "Util.h"

auto MapNormalizer::getProvinceType(std::uint32_t color) -> ProvinceType {
    if(color == RED_MASK) {
        return ProvinceType::LAND;
    } else if(color == BLUE_MASK) {
            return ProvinceType::SEA;
    } else if(color == GREEN_MASK) {
            return ProvinceType::LAKE;
    } else {
            return ProvinceType::UNKNOWN;
    }
}

auto MapNormalizer::getProvinceType(const Color& color) -> ProvinceType {
    return getProvinceType(colorToRGB(color));
}

bool MapNormalizer::isCoastal(const Color& color) {
    // Masks the value to just have coastal bit
    auto value = colorToRGB(color) & PROV_COASTAL_MASK;

    // Shift down until the least significant bit is the first one
    //  It should be the only bit remaining
    return (value >> indexOfLSB(value)) == 1;
}

auto MapNormalizer::getTerrainType(const Color& color) -> TerrainID {
    auto value = colorToRGB(color) & PROV_TERRAIN_MASK;

    return std::to_string(static_cast<uint8_t>(value >> indexOfLSB(value)));
}

auto MapNormalizer::getContinent(const Color& color) -> Continent {
    auto value = colorToRGB(color) & PROV_CONTINENT_ID_MASK;

    return std::to_string(static_cast<uint8_t>(value >> indexOfLSB(value)));
}

auto MapNormalizer::getState(const Color& color) -> StateID {
    auto value = colorToRGB(color) & PROV_STATE_ID_MASK;

    // No need for a shift, since states are already at the LSB
    return value;
}

MapNormalizer::ProvinceList MapNormalizer::createProvinceList(const PolygonList& shape_list)
{
    ProvinceList provinces;

    for(ProvinceID i = 0; i < shape_list.size(); ++i) {
        auto&& shape = shape_list[i];

        Color color = shape.color;

        auto prov_type = getProvinceType(color);

        auto is_coastal = isCoastal(color);
        auto terrain_type = getTerrainType(color);
        auto continent = getContinent(color);
        auto state = getState(color);

        auto&& bounding_box = shape.bounding_box;

        provinces.push_back(Province{
            i + 1, shape.unique_color,
            prov_type, is_coastal, terrain_type, continent, state, bounding_box,
            { }
        });
    }

    return provinces;
}

std::ostream& MapNormalizer::operator<<(std::ostream& stream,
                                        const MapNormalizer::Province& province)
{
    stream << province.id << ';' << static_cast<int>(province.unique_color.r)
           << ';' << static_cast<int>(province.unique_color.g) << ';'
           << static_cast<int>(province.unique_color.b) << ';'
           << province.type << ';' << (province.coastal ? "true" : "false")
           << ';' << province.terrain << ';'
           << province.continent;

    return stream;
}

