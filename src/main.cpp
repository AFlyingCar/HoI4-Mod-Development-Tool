
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <thread>

#include "BitMap.h" // BitMap
#include "Types.h" // Point, Color, Polygon, Pixel
#include "ShapeFinder.h" // findAllShapes
#include "GraphicalDebugger.h" // graphicsWorker
#include "ProvinceMapBuilder.h"

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "Missing filename argument." << std::endl;
    }

    if(argc < 3) {
        std::cerr << "Missing output directory argument." << std::endl;
        return 1;
    }

    char* filename = argv[1];
    char* outdirname = argv[2];

    MapNormalizer::BitMap* image = MapNormalizer::readBMP(filename);

    if(image == nullptr) {
        std::cerr << "Reading bitmap failed." << std::endl;
        return 1;
    }

    std::cout << "Read a bitmap of " << image->info_header.width << " by "
              << image->info_header.height << " pixels." << std::endl;

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

    std::thread graphics_thread([&image, &done, &graphics_data]() {
            MapNormalizer::graphicsWorker(image, graphics_data, done);
    });

    auto shapes = MapNormalizer::findAllShapes(image, graphics_data);

    std::cout << "Detected " << shapes.size() << " shapes." << std::endl;

    for(size_t i = 0; i < shapes.size(); ++i) {
        auto color = shapes.at(i).color;
        auto c = (color.r << 16) | (color.g << 8) | color.b;
        std::cout << std::dec << i << ": " << shapes.at(i).pixels.size();
        std::cout << " pixels, color = 0x" << std::hex << c << std::endl;
    }

    auto provinces = MapNormalizer::createProvinceList(shapes);

    std::filesystem::path output_path(outdirname);
    std::ofstream output_csv(output_path / "definition.csv");

    std::cout << "Provinces CSV:\n"
                 "=============="
              << std::endl;
    for(size_t i = 0; i < provinces.size(); ++i) {
        std::cout << std::dec << provinces[i] << std::endl;
        output_csv << std::dec << provinces[i] << std::endl;
    }

    std::cout << "Writing province bitmap to file..." << std::endl;
    MapNormalizer::writeBMP(output_path / "provinces.bmp", graphics_data,
                            image->info_header.width, image->info_header.height);
    std::cout << "Done." << std::endl;

    std::cout << "Press any key to exit.";

    std::getchar();
    done = true;

    graphics_thread.join();

    // Note: no need to free graphics_data, since we are exiting anyway and the
    //   OS will clean it up for us.

    return 0;
}

