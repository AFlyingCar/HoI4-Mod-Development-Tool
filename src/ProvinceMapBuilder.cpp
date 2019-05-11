
#include "ProvinceMapBuilder.h"

#include "UniqueColorGenerator.h"

MapNormalizer::ProvinceType MapNormalizer::getProvinceType(const Color& color) {
    // TODO
    return ProvinceType::UNKNOWN;
}

bool MapNormalizer::isCoastal(const Polygon& shape) {
    // TODO
    return false;
}

MapNormalizer::Terrain MapNormalizer::getTerrainType(const Color& color) {
    // TODO
    return Terrain::UNKNOWN;
}

size_t MapNormalizer::getContinent(const Color& color) {
    // TODO
    return 1;
}

MapNormalizer::ProvinceList MapNormalizer::createProvinceList(const PolygonList& shape_list)
{
    ProvinceList provinces;

    for(size_t i = 0; i < shape_list.size(); ++i) {
        auto&& shape = shape_list[i];

        provinces.push_back(Province{
            i,
            generateUniqueColor(i),
            getProvinceType(shape.color),
            isCoastal(shape),
            getTerrainType(shape.color),
            getContinent(shape.color)
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
           << province.type << ';' << province.coastal << ';'
           << province.terrain << ';' << province.continent;

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

