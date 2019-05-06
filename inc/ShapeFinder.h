#ifndef SHAPEFINDER_H
#define SHAPEFINDER_H

#include "Types.h"
#include "BitMap.h"

namespace MapNormalizer {
    bool isAdjacent(const Pixel&, size_t, size_t);
    bool isBoundaryPixel(Pixel);
    bool isInImage(BitMap*, uint32_t, uint32_t);
    uint32_t xyToIndex(BitMap*, uint32_t, uint32_t);
    uint32_t xyToIndex(uint32_t, uint32_t, uint32_t);
    Pixel getAsPixel(BitMap*, uint32_t, uint32_t);

    PolygonList findAllShapes(BitMap*, unsigned char* = nullptr);
}

#endif

