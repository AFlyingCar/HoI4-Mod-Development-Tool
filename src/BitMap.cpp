
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
    if(!safeRead(&(bm->header.filetype), file)        ||
       !safeRead(&(bm->header.fileSize), file)        ||
       !safeRead(&(bm->header.reserved1), file)       ||
       !safeRead(&(bm->header.reserved2), file)       ||
       !safeRead(&(bm->header.bitmapOffset), file)    ||
       !safeRead(&(bm->headerSize), file)      ||
       !safeRead(&(bm->width), file)           ||
       !safeRead(&(bm->height), file)          ||
       !safeRead(&(bm->bitPlanes), file)       ||
       !safeRead(&(bm->bitsPerPixel), file)    ||
       !safeRead(&(bm->compression), file)     ||
       !safeRead(&(bm->sizeOfBitmap), file)    ||
       !safeRead(&(bm->horzResolution), file)  ||
       !safeRead(&(bm->vertResolution), file)  ||
       !safeRead(&(bm->colorsUsed), file)      ||
       !safeRead(&(bm->colorImportant), file))
    {
        // Uh oh, one of the reads failed, delete the buffer and return nullptr
        delete bm;
        return nullptr;
    }

    // Allocate space for our new image data
    bm->data = new unsigned char[bm->sizeOfBitmap];

    // Make sure we read where the file tells us the offset is, and not just
    //  assume that the data starts after the header (it doesn't always do that)
    file.seekg(bm->header.bitmapOffset, file.beg);

    for(size_t i = 0; i < bm->sizeOfBitmap; i += 3) {
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

    WRITE_BMP_VALUE(header.filetype);
    WRITE_BMP_VALUE(header.fileSize);
    WRITE_BMP_VALUE(header.reserved1);
    WRITE_BMP_VALUE(header.reserved2);
    WRITE_BMP_VALUE(header.bitmapOffset);
    WRITE_BMP_VALUE(headerSize);
    WRITE_BMP_VALUE(width);
    WRITE_BMP_VALUE(height);
    WRITE_BMP_VALUE(bitPlanes);
    WRITE_BMP_VALUE(bitsPerPixel);
    WRITE_BMP_VALUE(compression);
    WRITE_BMP_VALUE(sizeOfBitmap);
    WRITE_BMP_VALUE(horzResolution);
    WRITE_BMP_VALUE(vertResolution);
    WRITE_BMP_VALUE(colorsUsed);
    WRITE_BMP_VALUE(colorImportant);

    // Pointers must be handled as a special case
    file.write(reinterpret_cast<const char*>(bmp->data), bmp->sizeOfBitmap);
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

    auto num_pixels = width * height;

    // bmp->filename = filename.c_str();
    bmp->header.filetype = BM_TYPE;
    bmp->header.fileSize = sizeof(BitMap);
    bmp->header.reserved1 = 0;
    bmp->header.reserved2 = 0;
    bmp->header.bitmapOffset = static_cast<uint32_t>(offsetof(BitMap, data));
    bmp->headerSize = sizeof(BitMapHeader);
    bmp->width = static_cast<int>(width);
    bmp->height = static_cast<int>(height);
    bmp->bitPlanes = 0;
    bmp->bitsPerPixel = 24; // 3 color values, 8 bits each
    bmp->compression = 0;
    bmp->sizeOfBitmap = num_pixels * 3; // 3 bytes per pixel
    bmp->horzResolution = 0; // TODO: Do we need to set this?
    bmp->vertResolution = 0; // TODO: Do we need to set this?
    bmp->colorsUsed = 0;
    bmp->colorImportant = 0;
    bmp->data = data;

    writeBMP(filename, bmp);

    delete bmp;
}

