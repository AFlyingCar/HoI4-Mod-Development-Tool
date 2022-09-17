/**
 * @file WorldNormalBuilder.h
 *
 * @brief Declares all functions that handle generating the world normal map.
 */

#ifndef WORLD_NORMAL_BUILDER_H
# define WORLD_NORMAL_BUILDER_H

# include <cstdint>

# include "Maybe.h"

namespace HMDT {
    struct BitMap;
    struct BitMap2;

    [[deprecated]] void generateWorldNormalMap(BitMap*, unsigned char*);

    MaybeVoid generateWorldNormalMap(const BitMap2&, unsigned char*);
}

#endif

