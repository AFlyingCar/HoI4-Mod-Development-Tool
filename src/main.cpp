
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
#include "Util.h"

int main(int argc, char** argv) {
    using namespace std::string_literals;

    if(argc < 2) {
        MapNormalizer::writeError("Missing filename argument.");
    }

    if(argc < 3) {
        MapNormalizer::writeError("Missing output directory argument.");
        return 1;
    }

    char* filename = argv[1];
    char* outdirname = argv[2];

    MapNormalizer::setInfoLine("Reading in .BMP file.");

    MapNormalizer::BitMap* image = MapNormalizer::readBMP(filename);

    if(image == nullptr) {
        MapNormalizer::writeError("Reading bitmap failed.");
        return 1;
    }

    std::cout << "BitMap = {" << std::endl;
    std::cout << "    Header = {" << std::endl;
    std::cout << "        filetype = " << image->file_header.filetype << std::endl;
    std::cout << "        fileSize = " << image->file_header.fileSize << std::endl;
    std::cout << "        reserved1 = " << image->file_header.reserved1 << std::endl;
    std::cout << "        reserved2 = " << image->file_header.reserved2 << std::endl;
    std::cout << "        bitmapOffset = " << image->file_header.bitmapOffset << std::endl;
    std::cout << "    }" << std::endl;
    std::cout << "    headerSize = " << image->info_header.headerSize << std::endl;
    std::cout << "    width = " << image->info_header.width << std::endl;
    std::cout << "    height = " << image->info_header.height << std::endl;
    std::cout << "    bitPlanes = " << image->info_header.bitPlanes << std::endl;
    std::cout << "    bitsPerPixel = " << image->info_header.bitsPerPixel << std::endl;
    std::cout << "    compression = " << image->info_header.compression << std::endl;
    std::cout << "    sizeOfBitmap = " << image->info_header.sizeOfBitmap << std::endl;
    std::cout << "    horzResolution = " << image->info_header.horzResolution << std::endl;
    std::cout << "    vertResolution = " << image->info_header.vertResolution << std::endl;
    std::cout << "    colorsUsed = " << image->info_header.colorsUsed << std::endl;
    std::cout << "    colorImportant = " << image->info_header.colorImportant << std::endl;
    std::cout << "    data = { ... }" << std::endl;
    std::cout << "}" << std::endl;

    unsigned char* graphics_data = nullptr;

    bool done = false;
    graphics_data = new unsigned char[image->info_header.width * image->info_header.height * 3];

#ifdef ENABLE_GRAPHICS
    MapNormalizer::writeDebug("Graphical debugger enabled.");
    std::thread graphics_thread([&image, &done, &graphics_data]() {
            MapNormalizer::graphicsWorker(image, graphics_data, done);
    });
#endif

    MapNormalizer::setInfoLine("Finding all possible shapes.");
    auto shapes = MapNormalizer::findAllShapes(image, graphics_data);

    MapNormalizer::setInfoLine("");
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
        ss << "\t{\n"
           << "\t\t(" << problem_pixel.point.x << ',' << problem_pixel.point.y << ")\n"
           << "\t\t0x" << std::hex << colorToRGB(problem_pixel.color) << "\n"
           << "\t}\n";
        MapNormalizer::writeWarning(ss.str(), false);
    }

    MapNormalizer::setInfoLine("Creating Provinces List.");
    auto provinces = MapNormalizer::createProvinceList(shapes);
    std::filesystem::path output_path(outdirname);
    std::ofstream output_csv(output_path / "definition.csv");

    // std::cout << "Provinces CSV:\n"
    //              "=============="
    //           << std::endl;
    for(size_t i = 0; i < provinces.size(); ++i) {
        // std::cout << std::dec << provinces[i] << std::endl;
        output_csv << std::dec << provinces[i] << std::endl;
    }

    MapNormalizer::setInfoLine("Writing province bitmap to file...");
    MapNormalizer::writeBMP(output_path / "provinces.bmp", graphics_data,
                            image->info_header.width, image->info_header.height);

    MapNormalizer::setInfoLine("Press any key to exit.");

    std::getchar();
    done = true;

#ifdef ENABLE_GRAPHICS
    MapNormalizer::setInfoLine("Waiting for graphical debugger thread to join...");
    graphics_thread.join();
#endif

    // Note: no need to free graphics_data, since we are exiting anyway and the
    //   OS will clean it up for us.

    return 0;
}

