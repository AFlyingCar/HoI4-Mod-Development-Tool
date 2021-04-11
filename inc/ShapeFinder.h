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
    bool isAdjacent(const Pixel&, size_t, size_t);
    bool isBoundaryPixel(Pixel);

    extern std::uint32_t error_count;
    extern std::vector<Pixel> problematic_pixels;
    PolygonList findAllShapes(BitMap*, unsigned char*, unsigned char*,
                              std::vector<Pixel>& = problematic_pixels);
}

#endif

