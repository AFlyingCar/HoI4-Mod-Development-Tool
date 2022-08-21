/**
 * @file BitMap.cpp
 *
 * @brief Defines all functions related to reading and writing bitmap images.
 */

#include "BitMap.h"

#include <cstring>
#include <cstddef>
#include <cerrno>
#include <iostream>
#include <fstream>
#include <string>

#include "Constants.h"
#include "Logger.h"
#include "Util.h"
#include "Maybe.h"
#include "StatusCodes.h"

HMDT::MaybeVoid flipImage(unsigned char* output, unsigned char* input,
                          size_t pitch, int height) noexcept
{
    // Big enough to store exactly one line of the image
    unsigned char* temp = nullptr;
    try {
        temp = new unsigned char[pitch];
    } catch(const std::bad_alloc& e) {
        WRITE_ERROR("Failed to allocate enough space for one line of pixels (",
                    pitch, " bytes required): ", e.what());
        return HMDT::STATUS_BADALLOC;
    }

    size_t idx_s = 0;
    size_t idx_t = (height - 1) * pitch;
    for(size_t y = 0; y < height / 2; ++y) {
        std::memcpy(temp, input + idx_s, pitch);
        std::memcpy(output + idx_s, input + idx_t, pitch);
        std::memcpy(output + idx_t, temp, pitch);

        idx_s += pitch;
        idx_t -= pitch;
    }

    return HMDT::STATUS_SUCCESS;
}

/**
 * @brief Reads a bitmap file.
 *
 * @param path The path to read from.
 * @return A pointer to a bitmap struct if the bitmap was successfully read,
 *         nullptr otherwise.
 */
HMDT::BitMap* HMDT::readBMP(const std::filesystem::path& path) {
    BitMap* bm = new BitMap();

    if(readBMP(path, bm) == nullptr) {
        delete bm;
        return nullptr;
    }

    return bm;
}

/**
 * @brief Reads a bitmap file.
 *
 * @param path The path to read from.
 * @param bm The BitMap structure to write into
 * @return A pointer to a bitmap struct if the bitmap was successfully read,
 *         nullptr otherwise.
 */
HMDT::BitMap* HMDT::readBMP(const std::filesystem::path& path, BitMap* bm) {
    std::ifstream file(path, std::ios::in | std::ios::binary);

    if(!file.is_open()) {
        return nullptr;
    }

    return readBMP(file, bm);
}

/**
 * @brief Reads a bitmap file.
 *
 * @param file The istream containing a valid BMP file
 * @param bm The BitMap structure to write into
 * @return A pointer to a bitmap struct if the bitmap was successfully read,
 *         nullptr otherwise.
 */
HMDT::BitMap* HMDT::readBMP(std::istream& file, BitMap* bm) {
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
        return nullptr;
    }

    size_t depth = bm->info_header.bitsPerPixel / 8;

    // Calculate how many bytes make up one line
    size_t orig_pitch = bm->info_header.width * bm->info_header.bitsPerPixel;
    size_t new_pitch = bm->info_header.width * depth; // This is how many we _want_ each line to take up.

    // Allocate space for our new image data
    try {
        bm->data = new unsigned char[new_pitch * bm->info_header.height];
    } catch(const std::bad_alloc& e) {
        WRITE_ERROR("Failed to allocate enough space for the bitmap's data (",
                    new_pitch * bm->info_header.height, " bytes required): ",
                    e.what());
        return nullptr;
    }

    // Make sure we read where the file tells us the offset is, and not just
    //  assume that the data starts after the header (it doesn't always do that)
    file.seekg(bm->file_header.bitmapOffset, file.beg);

    if(!safeRead(bm->data, bm->info_header.sizeOfBitmap, file)) {
        delete bm->data;

        return nullptr;
    }

    if(orig_pitch % 4 != 0) {
        WRITE_WARN("BitMap Width is not a multiple of 4! May contain up to ",
                   (orig_pitch / 4), " padding bytes.");
    }

    // Swap B and R every 3 pixels (because BitMap is a stupid format)
    for(size_t i = 2; i < bm->info_header.sizeOfBitmap; i += depth)
        std::swap(bm->data[i], bm->data[i - 2]);

    //----------------
    // Flip the entire image, because BitMap is a weird format.
    //----------------
    auto res = flipImage(bm->data, bm->data, new_pitch, bm->info_header.height);
    if(IS_FAILURE(res)) {
        return nullptr;
    }

    return bm;
}

/**
 * @brief Writes a bitmap file
 *
 * @param path The path to write to
 * @param bmp The BitMap object to write.
 */
void HMDT::writeBMP(const std::filesystem::path& path, const BitMap* bmp) {
    std::ofstream file(path, std::ios::out | std::ios::binary);

    if(!file) {
        std::cerr << "[ERR] ~ Failed to open output file " << path << ": " << strerror(errno) << std::endl;
        return;
    }

    // Helper macro to make the following code easier to read
#define WRITE_BMP_VALUE(MEMBER) \
    file.write(reinterpret_cast<const char*>(&(MEMBER)), \
               sizeof(MEMBER))

    WRITE_BMP_VALUE(bmp->file_header.filetype);
    WRITE_BMP_VALUE(bmp->file_header.fileSize);
    WRITE_BMP_VALUE(bmp->file_header.reserved1);
    WRITE_BMP_VALUE(bmp->file_header.reserved2);
    WRITE_BMP_VALUE(bmp->file_header.bitmapOffset);
    WRITE_BMP_VALUE(bmp->info_header.headerSize);
    WRITE_BMP_VALUE(bmp->info_header.width);
    WRITE_BMP_VALUE(bmp->info_header.height);
    WRITE_BMP_VALUE(bmp->info_header.bitPlanes);
    WRITE_BMP_VALUE(bmp->info_header.bitsPerPixel);
    WRITE_BMP_VALUE(bmp->info_header.compression);
    WRITE_BMP_VALUE(bmp->info_header.sizeOfBitmap);
    WRITE_BMP_VALUE(bmp->info_header.horzResolution);
    WRITE_BMP_VALUE(bmp->info_header.vertResolution);
    WRITE_BMP_VALUE(bmp->info_header.colorsUsed);
    WRITE_BMP_VALUE(bmp->info_header.colorImportant);

    //----------------
    // Flip the entire image, because BitMap is a weird format.
    //----------------

    // This is how much space a single line takes up
    size_t depth = bmp->info_header.bitsPerPixel / 8;
    size_t pitch = bmp->info_header.width * depth;

    WRITE_DEBUG("Flipping entire image before we write it.");
    std::unique_ptr<unsigned char[]> output(new unsigned char[bmp->info_header.sizeOfBitmap]{ 0 });
    auto res = flipImage(output.get(), bmp->data, pitch, bmp->info_header.height);

    if(IS_FAILURE(res)) {
        WRITE_ERROR("Failed to flip image!");
    } else {
        WRITE_DEBUG("Image flipped successfully, writing to file.");
        file.write(reinterpret_cast<const char*>(output.get()),
                   bmp->info_header.sizeOfBitmap);
    }
}

/**
 * @brief Writes a bitmap file
 *
 * @param path The path to write to
 * @param data The color data to write
 * @param width The width of the bitmap
 * @param height The height of the bitmap
 */
void HMDT::writeBMP(const std::filesystem::path& path, unsigned char* data,
                    uint32_t width, uint32_t height, uint16_t depth)
{
    BitMap bmp;
    std::memset(&bmp, 0, sizeof(BitMap));

    // Just hard-code these, they (should) never change
    constexpr auto fheader_size = 14;
    constexpr auto iheader_size = 40;

    // Remove the size of the pointer from the calculated size
    constexpr auto fiheader_size = fheader_size + iheader_size;
    auto num_pixels = width * height;

    // bmp->filename = filename.c_str();
    bmp.file_header.filetype = BM_TYPE;
    bmp.file_header.fileSize = fiheader_size + num_pixels * depth;
    bmp.file_header.reserved1 = 0;
    bmp.file_header.reserved2 = 0;
    bmp.file_header.bitmapOffset = static_cast<uint32_t>(fiheader_size);
    bmp.info_header.headerSize = iheader_size;
    bmp.info_header.width = static_cast<int>(width);
    bmp.info_header.height = static_cast<int>(height);
    bmp.info_header.bitPlanes = 1;
    bmp.info_header.bitsPerPixel = depth * 8; // 8 bits per pixel
    bmp.info_header.compression = 0; // For Win32 systems, this is BI_RGB
    bmp.info_header.sizeOfBitmap = num_pixels * depth;
    bmp.info_header.horzResolution = 0; // TODO: Do we need to set this?
    bmp.info_header.vertResolution = 0; // TODO: Do we need to set this?
    bmp.info_header.colorsUsed = 0;
    bmp.info_header.colorImportant = 0;
    bmp.data = data;

    writeBMP(path, &bmp);
}

/**
 * @brief Outputs a BitMap to an ostream
 *
 * @param stream The output stream to write to.
 * @param bm The BitMap to write.
 *
 * @return The given stream after writing the BitMap to it.
 */
std::ostream& HMDT::operator<<(std::ostream& stream, const HMDT::BitMap& bm) {
    stream << "BitMap = {" << std::endl
           << "    Header = {" << std::endl
           << "        filetype = " << bm.file_header.filetype << std::endl
           << "        fileSize = " << bm.file_header.fileSize << std::endl
           << "        reserved1 = " << bm.file_header.reserved1 << std::endl
           << "        reserved2 = " << bm.file_header.reserved2 << std::endl
           << "        bitmapOffset = " << bm.file_header.bitmapOffset << std::endl
           << "    }" << std::endl
           << "    headerSize = " << bm.info_header.headerSize << std::endl
           << "    width = " << bm.info_header.width << std::endl
           << "    height = " << bm.info_header.height << std::endl
           << "    bitPlanes = " << bm.info_header.bitPlanes << std::endl
           << "    bitsPerPixel = " << bm.info_header.bitsPerPixel << std::endl
           << "    compression = " << bm.info_header.compression << std::endl
           << "    sizeOfBitmap = " << bm.info_header.sizeOfBitmap << std::endl
           << "    horzResolution = " << bm.info_header.horzResolution << std::endl
           << "    vertResolution = " << bm.info_header.vertResolution << std::endl
           << "    colorsUsed = " << bm.info_header.colorsUsed << std::endl
           << "    colorImportant = " << bm.info_header.colorImportant << std::endl
           << "    data = { ... }" << std::endl
           << "}";

    return stream;
}

