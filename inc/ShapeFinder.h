/**
 * @file ShapeFinder.h
 *
 * @brief Defines functions for finding and validating shapes in an input .BMP.
 */

#ifndef SHAPEFINDER_H
# define SHAPEFINDER_H

# include "Types.h"
# include "BitMap.h"

namespace MapNormalizer {
    bool isShapeTooLarge(uint32_t, uint32_t, BitMap*);
    std::pair<uint32_t, uint32_t> calcShapeDims(const Polygon&);

    bool isAdjacent(const Pixel&, size_t, size_t);
    bool isBoundaryPixel(Pixel);
    bool isInImage(BitMap*, uint32_t, uint32_t);

    extern std::uint32_t error_count;
    extern std::vector<Pixel> problematic_pixels;
    PolygonList findAllShapes(BitMap*, unsigned char*, unsigned char*,
                              std::vector<Pixel>& = problematic_pixels);
}

#endif

