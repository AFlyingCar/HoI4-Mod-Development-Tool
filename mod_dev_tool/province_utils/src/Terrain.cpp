
#include "Terrain.h"

#include <map>
#include <fstream>
#include <cstdlib>

#include "Logger.h"
#include "Types.h"
#include "Util.h"

namespace {
    std::map<std::uint8_t, std::string> terrain_id_map;
    const std::string UNKNOWN_TERRAIN_NAME = "unknown";

    struct DefaultTerrains {
        DefaultTerrains():
            terrains { 
                { "unknown" },
                { "ocean" },
                { "lakes" },
                { "forest" },
                { "hills" },
                { "mountain" },
                { "plains" },
                { "urban" },
                { "jungle" },
                { "marsh" },
                { "desert" },
                { "water_fjords" },
                { "water_shallow_sea" },
                { "water_deep_ocean" }
            }
        { }

        std::vector<HMDT::Terrain> terrains;
    };
}

HMDT::Terrain::Terrain(const std::string& identifier):
    m_identifier(identifier)
{ }

const std::string& HMDT::Terrain::getIdentifier() const {
    return m_identifier;
}

const std::vector<HMDT::Terrain>& HMDT::getDefaultTerrains() {
    static DefaultTerrains defaults;

    return defaults.terrains;
}

