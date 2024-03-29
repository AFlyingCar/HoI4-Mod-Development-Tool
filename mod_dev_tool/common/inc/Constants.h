/**
 * @file Constants.h
 *
 * @brief Defines various constants used throughout the rest of the program.
 */

#ifndef CONSTANTS_H
# define CONSTANTS_H

# include <cstdint>
# include <string>

# include "Uuid.h"
# include "Types.h"
# include "Version.h"

namespace HMDT {
    //! Mask to get RED values out of a 24-bit color.
    const std::uint32_t RED_MASK   = 0xFF0000;
    //! Mask to get GREEN values out of a 24-bit color.
    const std::uint32_t GREEN_MASK = 0x00FF00;
    //! Mask to get BLUE values out of a 24-bit color.
    const std::uint32_t BLUE_MASK  = 0x0000FF;
    //! Mask for all color values in a 24-bit color.
    const std::uint32_t COLOR_MASK = 0xFFFFFF;

    //! The magic number marking that a file is a BitMap
    const std::uint16_t BM_TYPE = 19778; // BM

    //! The debug color for finding shapes.
    const Color DEBUG_COLOR = Color{ 255, 255, 255 };

    //! The debug color for the current cursor when finding shapes.
    const Color CURSOR_COLOR = Color{ 0, 0, 255 };

    //! The minimum number of pixels that can be in a valid province
    constexpr size_t MIN_SHAPE_SIZE = 8;

    //! The color of boundary pixels
    const Color BORDER_COLOR = Color{ 0, 0, 0 };

    //! The name of the application
    const std::string APPLICATION_NAME = "HoI4 Mod Development Tool";

    //! A simplified application name
    const std::string APPLICATION_SIMPLE_NAME = "hoi4_mod_dev_tool";

    //! The name of the status category
    const std::string_view STATUS_CATEGORY_NAME = "HMDT Status";

    //! The Gtk ID of the application
    const std::string APPLICATION_ID = "com.aflyingcar.tool.hoi4_mod_dev_tool";

    //! Default width of the properties pane, 30% of the minimum window width of 512
    const size_t MINIMUM_PROPERTIES_PANE_WIDTH = 185;

    //! The minimum width of the window
    const size_t MINIMUM_WINDOW_W = 512;

    //! The minimum height of the window
    const size_t MINIMUM_WINDOW_H = 512;

    //! The extension for project files
    const std::string PROJ_EXTENSION = ".hoi4proj";

    //! The folder name for metadata files about a project
    const std::string PROJ_META_FOLDER = ".projmeta";

    //! The filename for storing shape data
    const std::string SHAPEDATA_FILENAME = "shapedata.bin";

    //! The filename for storing data about provinces
    const std::string PROVINCEDATA_FILENAME = "definition.csv";

    //! The filename for storing the exported provinces bitmap
    const std::string PROVINCES_FILENAME = "provinces.bmp";

    //! The filename for storing data about continents
    const std::string CONTINENTDATA_FILENAME = "continents";

    //! The filename for storing the exported continents
    const std::string CONTINENT_FILENAME = "continent.txt";

    //! The filename for storing the heightmap
    const std::string HEIGHTMAP_FILENAME = "heightmap.bmp";

    //! The filename for storing the rivers
    const std::string RIVERS_FILENAME = "rivers.bmp";

    //! The filename for storing the normalmap
    const std::string NORMALMAP_FILENAME = "world_normal.bmp";

    //! The filename for storing the exported cities bitmap
    const std::string CITIESBMP_FILENAME = "cities.bmp";

    //! The filename for storing data about states
    const std::string STATEDATA_FILENAME = "states.csv";

    //! The filename for the imported province maps
    const std::string INPUT_PROVINCEMAP_FILENAME = "import_provincemap.bmp";

    //! The file extension for the log files
    const std::string LOG_FILE_EXTENSION = ".log";

    //! The file extension for config files
    const std::string CONF_FILE_EXTENSION = ".conf";

    //! The 4 magic bytes 
    const std::string SHAPEDATA_MAGIC = "SDAT";

    //! The maximum number of province previews to store in memory
    const size_t MAX_CACHED_PROVINCE_PREVIEWS = 100;

    //! How much to zoom each time
    const double ZOOM_FACTOR = 0.1;

    //! The default zoom level
    const double DEFAULT_ZOOM = 1.0;

    const uint32_t PROVINCE_HIGHLIGHT_COLOR = 0xFFFFFFFF;

    const std::string SOURCE_LOCATION = "https://github.com/AFlyingCar/HoI4-Mod-Development-Tool";

    //! The version of the tool
    extern const Version TOOL_VERSION;

    //! The license of the tool
    extern const std::string TOOL_LICENSE;

    //! The maximum number of provinces that can be selected
    const uint32_t MAX_SELECTED_PROVINCES = 128;

    //! The default value of buildings_max_level_factor
    const float DEFAULT_BUILDINGS_MAX_LEVEL_FACTOR = 1.0f;

    //! A completely impossible province ID that we will never support
    const ProvinceID INVALID_PROVINCE = EMPTY_UUID;
}

#endif

