
#ifndef BLANK_RIVER_MAP_BUILDER_H
#define BLANK_RIVER_MAP_BUILDER_H

#include "Types.h"
#include "BitMap.h"

namespace MapNormalizer {
    BitMap* buildBlankRiverMap(const ProvinceList&, const BitMap*,
                               const PolygonList&);
}

#endif

