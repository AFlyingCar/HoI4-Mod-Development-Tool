/**
 * @file UniqueColorGenerator.h
 *
 * @brief Defines how to get a unique color for a given province type.
 */

#ifndef UNIQUE_COLOR_GENERATOR_H
# define UNIQUE_COLOR_GENERATOR_H

# include "Types.h"

namespace HMDT {
    Color generateUniqueColor(ProvinceType);

    void resetUniqueColorGenerator(ProvinceType);
    void resetUniqueColorGenerator();
}

#endif

