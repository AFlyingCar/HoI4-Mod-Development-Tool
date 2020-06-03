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
#include "Util.h"

#include "ArgParser.h"
#include "Options.h"

MapNormalizer::ProgramOptions MapNormalizer::prog_opts;

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

    MapNormalizer::setInfoLine("Reading in .BMP file.");

    // Read the BitMap in
    MapNormalizer::BitMap* image = MapNormalizer::readBMP(MapNormalizer::prog_opts.infilename);

    if(image == nullptr) {
        MapNormalizer::writeError("Reading bitmap failed.");
        return 1;
    }

    if(MapNormalizer::prog_opts.verbose) {
        std::stringstream ss;
        ss << *image;
        MapNormalizer::writeDebug(ss.str(), false);
    }

    unsigned char* graphics_data = nullptr;
    unsigned char* river_data = nullptr;

    bool done = false;

    auto data_size = image->info_header.width * image->info_header.height * 3;

    graphics_data = new unsigned char[data_size];
    river_data = new unsigned char[data_size];

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

#if 0
    for(size_t i = 0; i < shapes.size(); ++i) {
        auto color = shapes.at(i).color;
        auto c = (color.r << 16) | (color.g << 8) | color.b;

        std::stringstream ss;
        ss << std::dec << i << ": " << shapes.at(i).pixels.size();
        ss << " pixels, color = 0x" << std::hex << c;

        MapNormalizer::writeDebug(ss.str());
    }
#endif

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

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Creating States List.");
    auto states = MapNormalizer::createStatesList(provinces, MapNormalizer::prog_opts.state_input_file);

    std::filesystem::path root_output_path(MapNormalizer::prog_opts.outpath);
    std::filesystem::path output_path = root_output_path / "map";
    std::filesystem::path state_output_root = root_output_path / "history";

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

    // std::cout << "Provinces CSV:\n"
    //              "=============="
    //           << std::endl;
    for(size_t i = 0; i < provinces.size(); ++i) {
        // std::cout << std::dec << provinces[i] << std::endl;
        output_csv << std::dec << provinces[i] << std::endl;
    }

    MapNormalizer::setInfoLine("Writing state definition files...");
    for(auto&& [state_id, state] : states) {
        // Don't bother to output states with no provinces in them
        if(!state.provinces.empty()) {
            auto filename = std::to_string(state_id) + " - " + state.name + ".txt";
            std::ofstream output_state(state_output_root / filename);

            output_state << state;
        }
    }

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Writing province bitmap to file...");
    MapNormalizer::writeBMP(output_path / "provinces.bmp", graphics_data,
                            image->info_header.width, image->info_header.height);

    if(!MapNormalizer::prog_opts.quiet)
        MapNormalizer::setInfoLine("Building blank river map...");
    MapNormalizer::writeBMP(output_path / "rivers.bmp", river_data,
                            image->info_header.width,
                            image->info_header.height);

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

