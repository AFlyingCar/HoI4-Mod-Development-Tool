#ifndef TYPES_H
#define TYPES_H

#include <vector> // std::vector
#include <cstdint> // uint32_t, uint8_t
#include <ostream>
#include <map>

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

    enum class Direction {
        NONE = 0,
        LEFT,
        UP,
        RIGHT,
        DOWN
    };

    /**
     * @brief A polygon, which may be a solid color shape and a vector of all
     *        pixels which make it up
     */
    struct Polygon {
        std::vector<Pixel> pixels;
        Color color; //!< Color of the shape as it was read in
        Color unique_color; //!< Unique color we have generated just for this shape

        // Used for checking to make sure the box isn't too big for HOI4
        Point2D bottom_left; //!< Bottom-left-most point in the polygon's bounding box
        Point2D top_right; //!< Top-right-most point in the polygon's bounding box
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

    using ProvinceID = std::uint32_t;
    using Terrain = std::uint8_t;
    using Continent = std::uint8_t;
    using StateID = std::uint32_t;

    /**
     * @brief A province as HOI4 will recognize it.
     */
    struct Province {
        ProvinceID id;
        Color unique_color;

        ProvinceType type;
        bool coastal;
        Terrain terrain;
        Continent continent;
        StateID state;
    };

    /**
     * @brief A list of all shapes
     */
    using PolygonList = std::vector<Polygon>;


    /**
     * @brief A list of all provinces
     */
    using ProvinceList = std::vector<Province>;

    /**
     * @brief A state as HOI4 will recognize it.
     */
    struct State {
        StateID id;
        std::string name;
        size_t manpower;
        std::string category;
        
        std::vector<ProvinceID> provinces;
    };

    /**
     * @brief A list of all states
     */
    using StateList = std::map<StateID, State>;
}

std::ostream& operator<<(std::ostream&, const MapNormalizer::Point2D&);
std::ostream& operator<<(std::ostream&, const MapNormalizer::Color&);
std::ostream& operator<<(std::ostream&, const MapNormalizer::ProvinceType&);
std::ostream& operator<<(std::ostream&, const MapNormalizer::State&);

std::string operator+(const std::string&, const MapNormalizer::Point2D&);
std::string operator+(const std::string&, const MapNormalizer::Color&);

bool operator==(const MapNormalizer::Color&, const MapNormalizer::Color&);
bool operator!=(const MapNormalizer::Color&, const MapNormalizer::Color&);

#endif

