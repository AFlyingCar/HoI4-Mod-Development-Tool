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
    class Terrain {
        public:
            Terrain(const std::string&);

            const std::string& getIdentifier() const;

        private:
            std::string m_identifier;
    };

    const std::vector<Terrain>& getDefaultTerrains();
}

#endif
