
#include "Interfaces.h"

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <thread>

// Common
#include "BitMap.h" // BitMap
#include "Types.h" // Point, Color, Polygon, Pixel
#include "Logger.h" // writeError
#include "Util.h"
#include "Options.h"

// GUI
#include "Driver.h"

// Exe
#include "ShapeFinder2.h" // findAllShapes2
#include "GraphicalDebugger.h" // graphicsWorker
#include "ProvinceMapBuilder.h"
#include "StateDefinitionBuilder.h"
#include "WorldNormalBuilder.h"

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
                writeDebug(ss.str());
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

int MapNormalizer::runHeadless() {
    std::filesystem::path root_output_path(prog_opts.outpath);
    std::filesystem::path output_path = root_output_path / "map";
    std::filesystem::path history_root = root_output_path / "history";
    std::filesystem::path state_output_root = history_root / "states";

    // If we were given a HoI4 install path, then generate override files
    //  Do this before anything else, in case we are asked to generate more state
    //  files that share a name with any vanilla ones
    if(!prog_opts.hoi4_install_path.empty()) {
        setInfoLine("Writing blank override files.");
        writeOverrideFiles(root_output_path,
                                          prog_opts.hoi4_install_path);
    }

    setInfoLine("Reading in .BMP file.");

    // Read the BitMap in
    BitMap* image = readBMP(prog_opts.infilename);

    if(image == nullptr) {
        writeError("Reading bitmap failed.");
        return 1;
    }

    BitMap* heightmap = nullptr;
    
    // Read in the heightmap only if we have one to read
    if(!prog_opts.heightmap_input_file.empty()) {
        heightmap = readBMP(prog_opts.heightmap_input_file);
    }

    if(prog_opts.verbose) {
        std::stringstream ss;
        ss << *image;
        writeDebug(ss.str(), false);
    }

    unsigned char* graphics_data = nullptr;
    unsigned char* river_data = nullptr;
    unsigned char* normalmap_data = nullptr;

    auto data_size = image->info_header.width * image->info_header.height * 3;

    {
        std::stringstream ss;
        ss << "Allocating " << data_size << " bytes of space for each output image.";
        writeDebug(ss.str());
    }

    river_data = new unsigned char[data_size];
    normalmap_data = new unsigned char[data_size];
    graphics_data = new unsigned char[data_size];

    if(!prog_opts.quiet)
        setInfoLine("Finding all possible shapes.");

    // Find every shape
    ShapeFinder shape_finder(image);
    auto shapes = shape_finder.findAllShapes();

    // Redraw the new image so we can properly show how it should look in the
    //  final output
    if(!prog_opts.quiet)
        setInfoLine("Drawing new graphical image");
    for(auto&& shape : shapes) {
        for(auto&& pixel : shape.pixels) {
            // Write to both the output data and into the displayed data
            writeColorTo(image->data, image->info_header.width,
                                        pixel.point.x, pixel.point.y,
                                        shape.unique_color);
        }
    }

    deleteInfoLine();

    if(!prog_opts.quiet)
        writeStdout("Detected ", std::to_string(shapes.size()), " shapes.");

    if(!prog_opts.quiet)
        setInfoLine("Creating Provinces List.");
    auto provinces = createProvinceList(shapes);

    StateList states;

    // Only produce a states list if we were given a state input file
    if(auto sif = prog_opts.state_input_file; !sif.empty()) {
        if(!prog_opts.quiet)
            setInfoLine("Creating States List.");
        states = createStatesList(provinces, sif);
    }

    // Generate the normal map if we have been given a height map
    if(heightmap != nullptr)
        generateWorldNormalMap(heightmap, normalmap_data);

    if(!std::filesystem::exists(output_path)) {
        using namespace std::string_literals;
        writeStdout("Path '", output_path.generic_string(), "' does not exist, creating...");
        std::filesystem::create_directories(output_path);
    }

    if(!std::filesystem::exists(state_output_root)) {
        using namespace std::string_literals;
        writeStdout("Path '", state_output_root.generic_string(), "' does not exist, creating...");
        std::filesystem::create_directories(state_output_root);
    }

    setInfoLine("Writing province definition file...");
    std::ofstream output_csv(output_path / "definition.csv");

    for(size_t i = 0; i < provinces.size(); ++i) {
        output_csv << std::dec << provinces[i] << std::endl;
    }

    // Only produce each state definition file if there are actually states to
    //  produce
    if(!states.empty()) {
        setInfoLine("Writing state definition files...");
        for(auto&& [state_id, state] : states) {
            // Do not write states that do not have a name unless --no-skip-no-name-state was passed
            if(state.name.empty()) {
                std::stringstream ss;
                ss << "State " << state_id << " does not have a name defined.";

                if(prog_opts.no_skip_no_name_state) {
                    writeWarning(ss.str());
                } else {
                    ss << " Skipping...";
                    writeError(ss.str());
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

    if(!prog_opts.quiet)
        setInfoLine("Writing province bitmap to file...");
    writeBMP(output_path / "provinces.bmp", image->data,
                            image->info_header.width, image->info_header.height);

    if(!prog_opts.quiet)
        setInfoLine("Writing blank river bitmap to file...");
    writeBMP(output_path / "rivers.bmp", river_data,
                            image->info_header.width,
                            image->info_header.height);

    // Write the new world_normal map to a file
    if(heightmap != nullptr) {
        if(!prog_opts.quiet)
            setInfoLine("Writing normal bitmap to file...");
        writeBMP(output_path / "world_normal.bmp", normalmap_data,
                                heightmap->info_header.width,
                                heightmap->info_header.height);
    }

    setInfoLine("Press any key to exit.");

    std::getchar();

    // One last newline so that the command line isn't horrible at the end of
    //   all this
    std::cout << std::endl;

    // Note: no need to free graphics_data, since we are exiting anyway and the
    //   OS will clean it up for us.

    return 0;
}

int MapNormalizer::runGUIApplication() {
    auto& driver = GUI::Driver::getInstance();

    driver.initialize();

    driver.run();

    return 0;
}

int MapNormalizer::runApplication() {
    if(prog_opts.headless) {
        return runHeadless();
    } else {
        return runGUIApplication();
    }
}

