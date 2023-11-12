#ifndef PROJECT_HIERARCHY_NODEKEYNAMES_H
# define PROJECT_HIERARCHY_NODEKEYNAMES_H

# include "PreprocessorUtils.h"

namespace HMDT::Project::Hierarchy {
    /**
     * @brief Defines all keys for Project nodes
     */
    struct ProjectKeys {
        static constexpr const char* MAP = HMDT_LOCALIZE("Map");
        static constexpr const char* PROVINCES = HMDT_LOCALIZE("Provinces");
        static constexpr const char* STATES = HMDT_LOCALIZE("States");
        static constexpr const char* CONTINENT = HMDT_LOCALIZE("Continent");
        static constexpr const char* HEIGHT_MAP = HMDT_LOCALIZE("HeightMap");
        static constexpr const char* HISTORY = HMDT_LOCALIZE("History");
        static constexpr const char* RIVERS = HMDT_LOCALIZE("Rivers");
    };

    /**
     * @brief Defines all keys for Group nodes
     */
    struct GroupKeys {
        static constexpr const char* PROVINCES = HMDT_LOCALIZE("Provinces");
        static constexpr const char* STATES = HMDT_LOCALIZE("States");
        static constexpr const char* CONTINENTS = HMDT_LOCALIZE("Continents");
    };

    /**
     * @brief Defines all keys for state property nodes
     */
    struct StateKeys {
        static constexpr const char* const ID = HMDT_LOCALIZE("ID");
        static constexpr const char* const MANPOWER = HMDT_LOCALIZE("Manpower");
        static constexpr const char* const CATEGORY = HMDT_LOCALIZE("Category");
        static constexpr const char* const BUILDINGS_MAX_LEVEL_FACTOR = HMDT_LOCALIZE("BuildingsMaxLevelFactor");
        static constexpr const char* const IMPASSABLE = HMDT_LOCALIZE("Impassable");
        static constexpr const char* const PROVINCES = HMDT_LOCALIZE("Provinces");
    };

    /**
     * @brief Defines all keys for province property nodes
     */
    struct ProvinceKeys {
        static constexpr const char* const ID = HMDT_LOCALIZE("ID");
        static constexpr const char* const COLOR = HMDT_LOCALIZE("Color");
        static constexpr const char* const TYPE = HMDT_LOCALIZE("Type");
        static constexpr const char* const COASTAL = HMDT_LOCALIZE("Coastal");
        static constexpr const char* const TERRAIN = HMDT_LOCALIZE("Terrain");
        static constexpr const char* const CONTINENT = HMDT_LOCALIZE("Continent");
        static constexpr const char* const STATE = HMDT_LOCALIZE("State");
        static constexpr const char* const ADJACENT_PROVINCES = HMDT_LOCALIZE("AdjacentProvinces");
    };
}

#endif

