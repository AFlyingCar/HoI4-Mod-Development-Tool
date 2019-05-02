#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <cstdint>

namespace MapNormalizer {
    struct Point2D {
        uint32_t x;
        uint32_t y;
    };

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    struct Pixel {
        Point2D point;
        Color color;
    };

    struct Polygon {
        std::vector<Pixel> pixels;
        Color u_color; // unique color value
    };

    using PolygonList = std::vector<Polygon>;
}

#endif

