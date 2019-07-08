
#include "BitMap.h"

#include <cstring>
#include <cstddef>
#include <cerrno>
#include <iostream>

#include "Constants.h"

MapNormalizer::BitMap* MapNormalizer::readBMP(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if(!file.is_open()) {
        return nullptr;
    }

    BitMap* bm = new BitMap();

    // bm->filename = filename.c_str();

    // Safely read the entire header into the struct.
    if(!safeRead(&(bm->file_header.filetype), file)        ||
       !safeRead(&(bm->file_header.fileSize), file)        ||
       !safeRead(&(bm->file_header.reserved1), file)       ||
       !safeRead(&(bm->file_header.reserved2), file)       ||
       !safeRead(&(bm->file_header.bitmapOffset), file)    ||
       !safeRead(&(bm->info_header.headerSize), file)      ||
       !safeRead(&(bm->info_header.width), file)           ||
       !safeRead(&(bm->info_header.height), file)          ||
       !safeRead(&(bm->info_header.bitPlanes), file)       ||
       !safeRead(&(bm->info_header.bitsPerPixel), file)    ||
       !safeRead(&(bm->info_header.compression), file)     ||
       !safeRead(&(bm->info_header.sizeOfBitmap), file)    ||
       !safeRead(&(bm->info_header.horzResolution), file)  ||
       !safeRead(&(bm->info_header.vertResolution), file)  ||
       !safeRead(&(bm->info_header.colorsUsed), file)      ||
       !safeRead(&(bm->info_header.colorImportant), file))
    {
        // Uh oh, one of the reads failed, delete the buffer and return nullptr
        delete bm;
        return nullptr;
    }

    // Allocate space for our new image data
    bm->data = new unsigned char[bm->info_header.sizeOfBitmap];

    // Make sure we read where the file tells us the offset is, and not just
    //  assume that the data starts after the header (it doesn't always do that)
    file.seekg(bm->file_header.bitmapOffset, file.beg);

    for(size_t i = 0; i < bm->info_header.sizeOfBitmap; i += 3) {
        unsigned char bgr[3];
        if(!safeRead(bgr, 3, file)) {
            // Uh oh, couldn't read the whole thing, this means the bitmap's
            //  header is wrong, or something else happened when reading.
            delete bm->data;
            delete bm;
            return nullptr;
        }

        bm->data[i]     = bgr[2];
        bm->data[i + 1] = bgr[1];
        bm->data[i + 2] = bgr[0];
    }

    return bm;
}

/**
 * @brief Writes a bitmap file
 *
 * @param filename The filename to write to
 * @param bmp The BitMap object to write.
 */
void MapNormalizer::writeBMP(const std::string& filename, const BitMap* bmp) {
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if(!file) {
        std::cerr << "[ERR] ~ Failed to open output file " << filename << ": " << strerror(errno) << std::endl;
        return;
    }

    // Helper macro to make the following code easier to read
#define WRITE_BMP_VALUE(MEMBER) \
    file.write(reinterpret_cast<const char*>(&(bmp->MEMBER)), \
               sizeof(bmp->MEMBER))

    WRITE_BMP_VALUE(file_header.filetype);
    WRITE_BMP_VALUE(file_header.fileSize);
    WRITE_BMP_VALUE(file_header.reserved1);
    WRITE_BMP_VALUE(file_header.reserved2);
    WRITE_BMP_VALUE(file_header.bitmapOffset);
    WRITE_BMP_VALUE(info_header.headerSize);
    WRITE_BMP_VALUE(info_header.width);
    WRITE_BMP_VALUE(info_header.height);
    WRITE_BMP_VALUE(info_header.bitPlanes);
    WRITE_BMP_VALUE(info_header.bitsPerPixel);
    WRITE_BMP_VALUE(info_header.compression);
    WRITE_BMP_VALUE(info_header.sizeOfBitmap);
    WRITE_BMP_VALUE(info_header.horzResolution);
    WRITE_BMP_VALUE(info_header.vertResolution);
    WRITE_BMP_VALUE(info_header.colorsUsed);
    WRITE_BMP_VALUE(info_header.colorImportant);

    // Pointers must be handled as a special case
    file.write(reinterpret_cast<const char*>(bmp->data), bmp->info_header.sizeOfBitmap);
}

/**
 * @brief Writes a bitmap file
 *
 * @param filename The filename to write to
 * @param data The color data to write
 * @param width The width of the bitmap
 * @param height The height of the bitmap
 */
void MapNormalizer::writeBMP(const std::string& filename, unsigned char* data,
                             uint32_t width, uint32_t height)
{
    BitMap* bmp = new BitMap();
    std::memset(bmp, 0, sizeof(BitMap));

    // Remove the size of the pointer from the calculated size
    auto bmp_size = sizeof(BitMap) - sizeof(char*);
    auto num_pixels = width * height;

    // bmp->filename = filename.c_str();
    bmp->file_header.filetype = BM_TYPE;
    bmp->file_header.fileSize = bmp_size + num_pixels * 3;
    bmp->file_header.reserved1 = 0;
    bmp->file_header.reserved2 = 0;
    bmp->file_header.bitmapOffset = static_cast<uint32_t>(offsetof(BitMap, data));
    bmp->info_header.headerSize = sizeof(BitMapInfoHeader);
    bmp->info_header.width = static_cast<int>(width);
    bmp->info_header.height = static_cast<int>(height);
    bmp->info_header.bitPlanes = 0;
    bmp->info_header.bitsPerPixel = 24; // 3 color values, 8 bits each
    bmp->info_header.compression = 0;
    bmp->info_header.sizeOfBitmap = num_pixels * 3; // 3 bytes per pixel
    bmp->info_header.horzResolution = 0; // TODO: Do we need to set this?
    bmp->info_header.vertResolution = 0; // TODO: Do we need to set this?
    bmp->info_header.colorsUsed = 0;
    bmp->info_header.colorImportant = 0;
    bmp->data = data;

    writeBMP(filename, bmp);

    delete bmp;
}

