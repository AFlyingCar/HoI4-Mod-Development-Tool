/**
 * @file ShapeFinder2.h
 *
 * @brief Defines functions for finding and validating shapes in an input .BMP.
 */

#ifndef SHAPEFINDER2_H
# define SHAPEFINDER2_H

# include "Types.h"
# include "BitMap.h"

# include <optional>

namespace MapNormalizer {
    std::pair<uint32_t, Color> getLabelAndColor(BitMap* image,
                                                const Point2D& point,
                                                uint32_t* label_matrix,
                                                const Color&);

    std::optional<Point2D> getAdjacentPixel(BitMap*, Point2D, Direction,
                                            Direction = Direction::NONE);

    void CCLPass1(BitMap*, uint32_t*, std::map<uint32_t, std::uint32_t>&);
    void CCLPass2(BitMap*, uint32_t*, const std::map<uint32_t, std::uint32_t>&);
    PolygonList CCLPass3(BitMap*, uint32_t*);

    PolygonList findAllShapes2(BitMap*);
}

#endif

