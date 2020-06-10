/**
 * @file main.cpp
 *
 * @brief The starting point of the program
 */

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <thread>

#include "BitMap.h" // BitMap
#include "Types.h" // Point, Color, Polygon, Pixel
#include "Logger.h" // writeError
#include "ShapeFinder.h" // findAllShapes
#include "GraphicalDebugger.h" // graphicsWorker
#include "ProvinceMapBuilder.h"
#include "StateDefinitionBuilder.h"
#include "WorldNormalBuilder.h"
#include "Util.h"

#include "ArgParser.h"
#include "Options.h"

MapNormalizer::ProgramOptions MapNormalizer::prog_opts;

// TODO: This should probably be moved into a different file
namespace MapNormalizer {
    /**
     * @brief Writes empty override files.
     *
     * @param root_output_path The root path to write the override files.
     * @param hoi4_install_path The root path to where HoI4 files are installed.
     * @param relative_path The path to where the files are stored that should
     *        be overridden, relative to both root_output_path and
     *        hoi4_install_path.
     */
    void writeOverrideFiles(const std::filesystem::path& root_output_path,
                            const std::filesystem::path& hoi4_install_path,
                            const std::filesystem::path& relative_path)
    {
        if(!std::filesystem::exists(root_output_path / relative_path))
            std::filesystem::create_directories(root_output_path / relative_path);

        std::filesystem::directory_iterator hoi4_di(hoi4_install_path / relative_path);
        for(auto&& dentry : hoi4_di) {
            // Do not do this for directories
            if(!dentry.is_directory()) {
                auto new_path = root_output_path / relative_path / dentry.path().filename();
                std::stringstream ss;
                ss << "Writing override file '" << new_path << '\'';
                MapNormalizer::writeDebug(ss.str());
                std::ofstream _(new_path);
            }
        }
    }

    /**
     * @brief Writes empty override files from the HoI4 install directory.
     *
     * @param root_output_path The root path to write the override files.
     * @param hoi4_install_path The root path to where HoI4 files are installed.
     */
    void writeOverrideFiles(const std::filesystem::path& root_output_path,
                            const std::filesystem::path& hoi4_install_path)
    {
        std::filesystem::path history = "history";
        std::filesystem::path map = "map";

        writeOverrideFiles(root_output_path, hoi4_install_path,
                           history / "states");
        writeOverrideFiles(root_output_path, hoi4_install_path,
                           history / "countries");
        writeOverrideFiles(root_output_path, hoi4_install_path,
                           history / "units");
        writeOverrideFiles(root_output_path, hoi4_install_path,
                           map / "strategicregions");
        writeOverrideFiles(root_output_path, hoi4_install_path,
                           map / "supplyareas");
    }
}

/**
 * @brief The starting point of the program.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 *
 * @return 1 upon failure, 0 upon success.
 */
int main(int argc, char** argv) {
    using namespace std::string_literals;

    // Parse the command-line arguments
    MapNormalizer::prog_opts = MapNormalizer::parseArgs(argc, argv);

    // Figure out if we should stop now based on the status of the parsing
    switch(MapNormalizer::prog_opts.status) {
        case 1:
            return 1;
        case 2:
            return 0;
        case 0:
        default:
            break;
    }

    std::filesystem::path root_output_path(MapNormalizer::prog_opts.outpath);
    std::filesystem::path output_path = root_output_path / "map";
    std::filesystem::path history_root = root_output_path / "history";
    std::filesystem::path state_output_root = history_root / "states";

    // If we were given a HoI4 install path, then generate override files
    //  Do this before anything else, in case we are asked to generate more state
    //  files that share a name with any vanilla ones
    if(!MapNormalizer::prog_opts.hoi4_install_path.empty()) {
        MapNormalizer::setInfoLine("Writing blank override files.");
        MapNormalizer::writeOverrideFiles(root_output_path,
                                          MapNormalizer::prog_opts.hoi4_install_path);
    }

    MapNormalizer::setInfoLine("Reading in .BMP file.");

    // Read the BitMap in
    MapNormalizer::BitMap* image = MapNormalizer::readBMP(MapNormalizer::prog_opts.infilename);

    if(image == nullptr) {
        MapNormalizer::writeError("Reading bitmap failed.");
        return 1;
    }

    MapNormalizer::BitMap* heightmap = nullptr;
    
    // Read in the heightmap only if we have one to read
    if(!MapNormalizer::prog_opts.heightmap_input_file.empty()) {
        heightmap = MapNormalizer::readBMP(MapNormalizer::prog_opts.heightmap_input_file);
    }

    if(MapNormalizer::prog_opts.verbose) {
        std::stringstream ss;
        ss << *image;
        MapNormalizer::writeDebug(ss.str(), false);
    }

    unsigned char* graphics_data = nullptr;
    unsigned char* river_data = nullptr;
    unsigned char* normalmap_data = nullptr;

    bool done = false;

    auto data_size = image->info_header.width * image->info_header.height * 3;

    {
        std::stringstream ss;
        ss << "Allocating " << data_size << " bytes of space for each output image.";
        MapNormalizer::writeDebug(ss.str());
    }

    graphics_data = new unsigned char[data_size];
    river_data = new unsigned char[data_size];
    normalmap_data = new unsigned char[data_size];

#ifdef ENABLE_GRAPHICS
    std::thread graphics_thread;
    if(!MapNormalizer::prog_opts.no_gui) {
        if(MapNormalizer::prog_opts.verbose)
            MapNormalizer::writeDebug("Graphical debugger enabled.");
        graphics_thread = std::thread([&image, &done, &graphics_data]() {
                MapNormalizer::graphicsWorker(image, graphics_data, done);
                // TODO: We should somehow switch the graphicsWorker over to
                //   displaying the blank river map builder
        });
    }
#endif

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Finding all possible shapes.");
    auto shapes = MapNormalizer::findAllShapes(image, graphics_data, river_data);

    MapNormalizer::setInfoLine("");

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::writeStdout("Detected "s + std::to_string(shapes.size()) + " shapes.");

    if(!MapNormalizer::problematic_pixels.empty())
        MapNormalizer::writeWarning("The following "s +
                                    std::to_string(MapNormalizer::problematic_pixels.size()) +
                                    " pixels had problems. This could be a bug "
                                    "with the program, or a problem with y our "
                                    "input file. Please check these pixels in "
                                    "your input in case of any problems.");
    for(auto& problem_pixel : MapNormalizer::problematic_pixels) {
        std::stringstream ss;
        ss << "\t{ (" << problem_pixel.point.x << ',' << problem_pixel.point.y
           << ") -> 0x" << std::hex << colorToRGB(problem_pixel.color) << " }";
        MapNormalizer::writeWarning(ss.str(), false);
    }

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Creating Provinces List.");
    auto provinces = MapNormalizer::createProvinceList(shapes);

    MapNormalizer::StateList states;

    // Only produce a states list if we were given a state input file
    if(auto sif = MapNormalizer::prog_opts.state_input_file; !sif.empty()) {
        if(!MapNormalizer::prog_opts.quiet)
            MapNormalizer::setInfoLine("Creating States List.");
        states = MapNormalizer::createStatesList(provinces, sif);
    }

    // Generate the normal map if we have been given a height map
    if(heightmap != nullptr)
        MapNormalizer::generateWorldNormalMap(heightmap, normalmap_data);

    if(!std::filesystem::exists(output_path)) {
        using namespace std::string_literals;
        MapNormalizer::writeStdout("Path '"s + output_path.generic_string() + "' does not exist, creating...");
        std::filesystem::create_directories(output_path);
    }

    if(!std::filesystem::exists(state_output_root)) {
        using namespace std::string_literals;
        MapNormalizer::writeStdout("Path '"s + state_output_root.generic_string() + "' does not exist, creating...");
        std::filesystem::create_directories(state_output_root);
    }

    MapNormalizer::setInfoLine("Writing province definition file...");
    std::ofstream output_csv(output_path / "definition.csv");

    for(size_t i = 0; i < provinces.size(); ++i) {
        output_csv << std::dec << provinces[i] << std::endl;
    }

    // Only produce each state definition file if there are actually states to
    //  produce
    if(!states.empty()) {
        MapNormalizer::setInfoLine("Writing state definition files...");
        for(auto&& [state_id, state] : states) {
            // Do not write states that do not have a name unless --no-skip-no-name-state was passed
            if(state.name.empty()) {
                std::stringstream ss;
                ss << "State " << state_id << " does not have a name defined.";

                if(MapNormalizer::prog_opts.no_skip_no_name_state) {
                    MapNormalizer::writeWarning(ss.str());
                } else {
                    ss << " Skipping...";
                    MapNormalizer::writeError(ss.str());
                    continue;
                }
            }

            // Don't bother to output states with no provinces in them
            if(!state.provinces.empty()) {
                auto filename = std::to_string(state_id) + "-" + state.name + ".txt";
                std::ofstream output_state(state_output_root / filename);

                output_state << state;
            }
        }
    }

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Writing province bitmap to file...");
    MapNormalizer::writeBMP(output_path / "provinces.bmp", graphics_data,
                            image->info_header.width, image->info_header.height);

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Writing blank river bitmap to file...");
    MapNormalizer::writeBMP(output_path / "rivers.bmp", river_data,
                            image->info_header.width,
                            image->info_header.height);

    // Write the new world_normal map to a file
    if(heightmap != nullptr) {
        if(!MapNormalizer::prog_opts.quiet)
            MapNormalizer::setInfoLine("Writing normal bitmap to file...");
        MapNormalizer::writeBMP(output_path / "world_normal.bmp", normalmap_data,
                                heightmap->info_header.width,
                                heightmap->info_header.height);
    }

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Press any key to exit.");

    std::getchar();
    done = true;

#ifdef ENABLE_GRAPHICS
    if(!MapNormalizer::prog_opts.no_gui) {
        if(!MapNormalizer::prog_opts.quiet)
            MapNormalizer::setInfoLine("Waiting for graphical debugger thread to join...");
        graphics_thread.join();
    }
#endif

    // One last newline so that the command line isn't horrible at the end of
    //   all this
    std::cout << std::endl;

    // Note: no need to free graphics_data, since we are exiting anyway and the
    //   OS will clean it up for us.

    return 0;
}

