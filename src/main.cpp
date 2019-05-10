
#include <iostream>
#include <algorithm>
#include <thread>

#include "BitMap.h" // BitMap
#include "Types.h" // Point, Color, Polygon, Pixel
#include "ShapeFinder.h" // findAllShapes
#include "GraphicalDebugger.h" // graphicsWorker

int main(int argc, char** argv) {
    if(argc < 2) {
        std::cerr << "Missing filename argument." << std::endl;
        std::cerr << "Missing output filename argument." << std::endl;
        return 1;
    }

    if(argc < 3) {
        std::cerr << "Missing output filename argument." << std::endl;
        return 1;
    }

    char* filename = argv[1];
    char* outfilename = argv[2];

    MapNormalizer::BitMap* image = MapNormalizer::readBMP(filename);

    if(image == nullptr) {
        std::cerr << "Reading bitmap failed." << std::endl;
        return 1;
    }

    std::cout << "Read a bitmap of " << image->width << " by " << image->height
              << " pixels." << std::endl;

    unsigned char* graphics_data = nullptr;

#ifdef ENABLE_GRAPHICS
    bool done = false;
    graphics_data = new unsigned char[image->width * image->height * 3];

    std::thread graphics_thread([&image, &done, &graphics_data]() {
            MapNormalizer::graphicsWorker(image, graphics_data, done);
    });
#endif

    auto shapes = MapNormalizer::findAllShapes(image, graphics_data);

    std::cout << "Detected " << shapes.size() << " shapes." << std::endl;

    for(size_t i = 0; i < shapes.size(); ++i) {
        auto color = shapes.at(i).u_color;
        auto c = (color.r << 16) | (color.g << 8) | color.b;
        std::cout << std::dec << i << ": " << shapes.at(i).pixels.size();
        std::cout << " pixels, color = 0x" << std::hex << c << std::endl;
    }

#ifdef ENABLE_GRAPHICS
    std::getchar();
    done = true;

    graphics_thread.join();
#endif

    return 0;
}
