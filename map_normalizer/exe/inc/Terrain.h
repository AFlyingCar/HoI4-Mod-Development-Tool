/**
 * @file Terrain.h
 *
 * @brief Defines all functions for dealing with external terrain information.
 */

#ifndef TERRAIN_H
# define TERRAIN_H

# include <filesystem>

# include "Types.h"

namespace MapNormalizer {
    void loadTerrainInfo(const std::filesystem::path&);
    void loadDefaultTerrainInfo();

    const std::string& getTerrainIdentifier(Terrain);
}

#endif
