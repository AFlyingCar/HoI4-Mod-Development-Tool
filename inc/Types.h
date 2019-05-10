#ifndef TYPES_H
#define TYPES_H

#include <vector> // std::vector
#include <cstdint> // uint32_t, uint8_t

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
        Color u_color; // TODO: This should be just the solid color fo this shape
        Color color; //!< Color of the shape as it was read in
    };

    /**
     * @brief A list of all shapes
     */
    using PolygonList = std::vector<Polygon>;
}

#endif

