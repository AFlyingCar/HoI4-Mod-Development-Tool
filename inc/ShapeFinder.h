#ifndef SHAPEFINDER_H
#define SHAPEFINDER_H

#include "Types.h" // Pixel
#include "BitMap.h" // BitMap

namespace MapNormalizer {
    constexpr size_t MIN_SHAPE_SIZE = 8;

    bool isShapeTooLarge(uint32_t, uint32_t, BitMap*);
    std::pair<uint32_t, uint32_t> calcShapeDims(const Polygon&);

    bool isAdjacent(const Pixel&, size_t, size_t);
    bool isBoundaryPixel(Pixel);
    bool doColorsMatch(Color, Color);
    bool isInImage(BitMap*, uint32_t, uint32_t);

    extern std::uint32_t error_count;
    extern std::vector<Pixel> problematic_pixels;
    PolygonList findAllShapes(BitMap*, unsigned char* = nullptr,
                              std::vector<Pixel>& = problematic_pixels);
}

#endif

