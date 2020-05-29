
#include "Terrain.h"

#include <map>
#include <fstream>
#include <cstdlib>

#include "Logger.h"
#include "Types.h"
#include "Util.h"

namespace {
    std::map<MapNormalizer::Terrain, std::string> terrain_id_map;
    const std::string UNKNOWN_TERRAIN_NAME = "unknown";
}

/*
 * Terrain Info format:
 *
 * <ID>=<NAME>
 * <ID2>=<NAME2>
 * ...
 */
void MapNormalizer::loadTerrainInfo(const std::filesystem::path& path) {
    using namespace std::string_literals;

    // NOTE: If we fail to load the terrain map info, then the map will be empty
    //  and we will end up loading from the default terrain list

    if(std::filesystem::exists(path)) {
        std::ifstream file(path);

        if(file) {
            std::string line;
            size_t lnum = 0;
            while(std::getline(file, line)) {
                ++lnum;

                // Skip blank lines
                if(line.empty()) continue;

                auto val_sep = line.find_first_of('=');

                // Stop parsing if the file is malformed
                if(val_sep == std::string::npos) {
                    writeError("Non-blank line ("s + std::to_string(lnum) + ") found missing '='.");
                    terrain_id_map.clear();
                    break;
                }

                auto id = line.substr(0, val_sep);
                auto name = line.substr(val_sep + 1);

                trim(id);
                trim(name);

                terrain_id_map[std::atoi(id.c_str())] = name;
            }
        } else {
            writeError("Failed to open terrain info file '"s + path.generic_string() + "'");
            terrain_id_map.clear();
        }
    } else {
        writeError("Failed to open terrain info file '"s + path.generic_string() + "': File does not exist.");
        terrain_id_map.clear();
    }
}

void MapNormalizer::loadDefaultTerrainInfo() {
    writeStdout("Loading default terrain information.");

    terrain_id_map[0] = "unknown";
    terrain_id_map[1] = "ocean";
    terrain_id_map[2] = "lakes";
    terrain_id_map[3] = "forest";
    terrain_id_map[4] = "hills";
	terrain_id_map[5] = "mountain";
	terrain_id_map[6] = "plains";
	terrain_id_map[7] = "urban";
	terrain_id_map[8] = "jungle";
	terrain_id_map[9] = "marsh";
	terrain_id_map[10] = "desert";
	terrain_id_map[11] = "water_fjords";
	terrain_id_map[12] = "water_shallow_sea";
	terrain_id_map[13] = "water_deep_ocean";
}

const std::string& MapNormalizer::getTerrainIdentifier(Terrain terrain) {
    // Go ahead and load the default info if we never were asked to load the real info
    if(terrain_id_map.empty()) {
        loadDefaultTerrainInfo();
    }

    return terrain_id_map.count(terrain) != 0 ? terrain_id_map.at(terrain) :
                                                UNKNOWN_TERRAIN_NAME;
}

