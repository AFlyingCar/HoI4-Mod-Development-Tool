
#include "ImageLoader.h"

#include <filesystem>

#include "BitMap.h"
#include "Logger.h"

MapNormalizer::GUI::ImageLoader::ImageLoader() {
}

MapNormalizer::GUI::ImageLoader::~ImageLoader() {
}

void* MapNormalizer::GUI::ImageLoader::loadImage(int& width, int& height,
                                                 MyGUI::PixelFormat& format,
                                                 const std::string& filename)
{
    std::filesystem::path path(filename);
    if(path.extension() == "bmp") {
        return loadBitMap(width, height, format, filename);
    } else {
        writeError("Unknown extension for image at path: ", filename);
        return nullptr;
    }
}

void MapNormalizer::GUI::ImageLoader::saveImage(int width, int height,
                                                MyGUI::PixelFormat format,
                                                void* texture,
                                                const std::string& filename)
{
    std::filesystem::path path(filename);
    if(path.extension() == "bmp") {
        return saveBitMap(width, height, format, texture, filename);
    } else {
        writeError("Unknown extension for image at path: ", filename);
    }
}

void* MapNormalizer::GUI::ImageLoader::loadBitMap(int& width, int& height,
                                                  MyGUI::PixelFormat& format,
                                                  const std::string& filename)
{
    BitMap bm;
    if(readBMP(filename, &bm) == nullptr) {
        return nullptr;
    }

    width = bm.info_header.width;
    height = bm.info_header.height;

    unsigned char* data = bm.data;

    return data;
}

void MapNormalizer::GUI::ImageLoader::saveBitMap(int width, int height,
                                                 MyGUI::PixelFormat format,
                                                 void* data,
                                                 const std::string& filename)
{
    // We don't actually do anything with the format here, should we?
    (void)format;

    writeBMP(filename, reinterpret_cast<unsigned char*>(data), width, height);
}

