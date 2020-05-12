#ifndef TYPES_H
#define TYPES_H

#include <vector> // std::vector
#include <cstdint> // uint32_t, uint8_t
#include <ostream>

namespace MapNormalizer {
    /**
     * @brief A 2D point
     */
    struct Point2D {
        uint32_t x;
        uint32_t y;
    };

    /**
     * @brief An RGB color value
     */
    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    /**
     * @brief A pixel, which is a point and color
     */
    struct Pixel {
        Point2D point;
        Color color;
    };

    /**
     * @brief A polygon, which may be a solid color shape and a vector of all
     *        pixels which make it up
     */
    struct Polygon {
        std::vector<Pixel> pixels;
        Color color; //!< Color of the shape as it was read in
        Color unique_color; //!< Unique color we have generated just for this shape
    };

    /**
     * @brief The type of province
     */
    enum class ProvinceType {
        UNKNOWN = 0,
        LAND,
        SEA,
        LAKE
    };

    /**
     * @brief The possible terrain types.
     */
    enum class Terrain {
        UNKNOWN = 0,
        DESERT,
        FOREST,
        HILLS,
        JUNGLE,
        MARSH,
        MOUNTAIN,
        PLAINS,
        URBAN,
        OCEAN,
        LAKE
    };

    /**
     * @brief A province as HOI4 will recognize it.
     */
    struct Province {
        size_t id;
        Color unique_color;
        ProvinceType type;
        bool coastal;
        Terrain terrain;
        size_t continent;
    };

    /**
     * @brief A list of all shapes
     */
    using PolygonList = std::vector<Polygon>;


    /**
     * @brief A list of all provinces
     */
    using ProvinceList = std::vector<Province>;
}

std::ostream& operator<<(std::ostream&, const MapNormalizer::Point2D&);
std::ostream& operator<<(std::ostream&, const MapNormalizer::Color&);
std::ostream& operator<<(std::ostream&, const MapNormalizer::ProvinceType&);

#endif

