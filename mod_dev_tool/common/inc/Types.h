#ifndef TYPES_H
# define TYPES_H

# include <vector> // std::vector
# include <cstdint> // uint32_t, uint8_t
# include <ostream>
# include <map>
# include <set>
# include <variant>
# include <optional>
# include <utility>
# include <unordered_map>

# include "Uuid.h"

namespace HMDT {
    /**
     * @brief A 2D point
     */
    struct Point2D {
        uint32_t x;
        uint32_t y;
    };

    /**
     * @brief Dimensions of an image
     */
    struct Dimensions {
        uint32_t w;
        uint32_t h;
    };

    /**
     * @brief A 2D Rectangle
     */
    struct Rectangle {
        uint32_t x;
        uint32_t y;
        uint32_t w;
        uint32_t h;
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
     * @brief Represents a direction along a 2D plane
     */
    enum class Direction {
        LEFT,
        UP,
        RIGHT,
        DOWN
    };

    /**
     * @brief An axis-aligned bounding box
     */
    struct BoundingBox {
        Point2D bottom_left; //!< Bottom-left-most point
        Point2D top_right; //!< Top-right-most point
    };

    /**
     * @brief A polygon, which may be a solid color shape and a vector of all
     *        pixels which make it up
     */
    struct Polygon {
        //! The ID of this polygon
        UUID id;

        std::vector<Pixel> pixels;
        Color color; //!< Color of the shape as it was read in
        Color unique_color; //!< Unique color we have generated just for this shape

        //! The bounding box of the polygon
        BoundingBox bounding_box;

        //! All adjacent shape labels
        std::set<UUID> adjacent_labels;
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

    using ProvinceID = UUID;
    using TerrainID = std::string;
    using Continent = std::string;
    using StateID = std::uint32_t;

    /**
     * @brief A province as HOI4 will recognize it.
     */
    struct Province {
        ProvinceID id;
        Color unique_color;

        ProvinceType type;
        bool coastal;
        TerrainID terrain;
        Continent continent;
        StateID state;

        BoundingBox bounding_box;

        std::set<ProvinceID> adjacent_provinces;

        //! The parent province
        ProvinceID parent_id;

        //! Child provinces
        std::set<ProvinceID> children;
    };

    /**
     * @brief A list of all shapes
     */
    using PolygonList = std::vector<Polygon>;


    /**
     * @brief A list of all provinces
     */
    using ProvinceList = std::unordered_map<ProvinceID, Province>;

    /**
     * @brief A state as HOI4 will recognize it.
     */
    struct State {
        StateID id;
        std::string name;
        size_t manpower;
        std::string category;
        float buildings_max_level_factor;
        bool impassable;
        
        std::vector<ProvinceID> provinces;

        Color color;

        // std::map<???, uint32_t> resources;
        // TODO: history?
    };

    /**
     * @brief A list of all states
     */
    using StateList = std::map<StateID, State>;

    std::ostream& operator<<(std::ostream&, const Point2D&);
    std::ostream& operator<<(std::ostream&, const Color&);

    std::ostream& operator<<(std::ostream&, const ProvinceType&);
    std::ostream& operator<<(std::ostream&, const State&);

    std::string operator+(const std::string&, const Point2D&);
    std::string operator+(const std::string&, const Color&);

    bool operator==(const Color&, const Color&);
    bool operator!=(const Color&, const Color&);

    /**
     * @brief Helper alias for std::reference_wrapper
     */
    template<typename T>
    using Ref = std::reference_wrapper<T>;

    /**
     * @brief A helper alias
     */
    template<typename T>
    using OptionalReference = std::optional<std::reference_wrapper<T>>;

    /**
     * @brief A helper alias
     */
    template<typename T>
    using RefVector = std::vector<std::reference_wrapper<T>>;
}

#endif

