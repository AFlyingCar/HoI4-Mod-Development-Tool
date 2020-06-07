/**
 * @file WorldNormalBuilder.h
 *
 * @brief Declares all functions that handle generating the world normal map.
 */

#ifndef WORLD_NORMAL_BUILDER_H
# define WORLD_NORMAL_BUILDER_H

namespace MapNormalizer {
    struct BitMap;

    void generateWorldNormalMap(BitMap*, unsigned char*);
}

#endif

