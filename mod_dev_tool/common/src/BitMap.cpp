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

#undef WRITE_BMP_VALUE
}

/**
 * @brief Writes a bitmap file
 *
 * @param path The path to write to
 * @param data The color data to write
 * @param width The width of the bitmap
 * @param height The height of the bitmap
 * @param depth The bit-depth of the bitmap
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

auto HMDT::readBMP(const std::filesystem::path& path,
                   std::shared_ptr<BitMap2> bm) noexcept
    -> MaybeRef<BitMap2>
{
    if(bm == nullptr) {
        RETURN_ERROR(STATUS_PARAM_CANNOT_BE_NULL);
    }

    return readBMP(path, *bm);
}

auto HMDT::readBMP(const std::filesystem::path& path, BitMap2& bm) noexcept
    -> MaybeRef<BitMap2>
{
    // Make sure we clear errno first
    errno = 0;

    std::ifstream file(path, std::ios::in | std::ios::binary);

    if(!file.is_open()) {
        WRITE_ERROR("Failed to open bitmap file ", path);
        RETURN_ERROR(std::error_code(errno, std::generic_category()));
    }

    auto res = readBMP(file, bm);
    RETURN_IF_ERROR(res);

    return res;
}

auto HMDT::readBMP(std::istream& stream, std::shared_ptr<BitMap2> bm) noexcept
    -> MaybeRef<BitMap2>
{
    if(bm == nullptr) {
       RETURN_ERROR(STATUS_PARAM_CANNOT_BE_NULL);
    }

    auto res = readBMP(stream, bm);
    RETURN_IF_ERROR(res);

    return res;
}

auto HMDT::readBMP(std::istream& stream, BitMap2& bm) noexcept
    -> MaybeRef<BitMap2>
{
#define READ_FROM_BMP(FIELD)                    \
    do {                                        \
        auto res = safeRead2(FIELD, stream); \
        RETURN_IF_ERROR(res);                   \
    } while(0)
#define READ_FROM_BMP2(FIELD, SIZE)                   \
    do {                                              \
        auto res = safeRead2(FIELD, SIZE, stream); \
        RETURN_IF_ERROR(res);                         \
    } while(0)

    // Safely read the entire header into the struct.
    READ_FROM_BMP(&(bm.file_header.filetype));
    READ_FROM_BMP(&(bm.file_header.fileSize));
    READ_FROM_BMP(&(bm.file_header.reserved1));
    READ_FROM_BMP(&(bm.file_header.reserved2));
    READ_FROM_BMP(&(bm.file_header.bitmapOffset));
    READ_FROM_BMP(&(bm.info_header.v1.headerSize));
    READ_FROM_BMP(&(bm.info_header.v1.width));
    READ_FROM_BMP(&(bm.info_header.v1.height));
    READ_FROM_BMP(&(bm.info_header.v1.bitPlanes));
    READ_FROM_BMP(&(bm.info_header.v1.bitsPerPixel));
    READ_FROM_BMP(&(bm.info_header.v1.compression));
    READ_FROM_BMP(&(bm.info_header.v1.sizeOfBitmap));
    READ_FROM_BMP(&(bm.info_header.v1.horzResolution));
    READ_FROM_BMP(&(bm.info_header.v1.vertResolution));
    READ_FROM_BMP(&(bm.info_header.v1.colorsUsed));
    READ_FROM_BMP(&(bm.info_header.v1.colorImportant));

    // Read the V4 part of the header
    if(bm.info_header.v1.headerSize >= 108) {
        READ_FROM_BMP(&(bm.info_header.v4.redMask));
        READ_FROM_BMP(&(bm.info_header.v4.greenMask));
        READ_FROM_BMP(&(bm.info_header.v4.blueMask));
        READ_FROM_BMP(&(bm.info_header.v4.alphaMask));
        READ_FROM_BMP(&(bm.info_header.v4.CSType));

        READ_FROM_BMP(&(bm.info_header.v4.redX));
        READ_FROM_BMP(&(bm.info_header.v4.redY));
        READ_FROM_BMP(&(bm.info_header.v4.redZ));
        READ_FROM_BMP(&(bm.info_header.v4.greenX));
        READ_FROM_BMP(&(bm.info_header.v4.greenY));
        READ_FROM_BMP(&(bm.info_header.v4.greenZ));
        READ_FROM_BMP(&(bm.info_header.v4.blueX));
        READ_FROM_BMP(&(bm.info_header.v4.blueY));
        READ_FROM_BMP(&(bm.info_header.v4.blueZ));

        READ_FROM_BMP(&(bm.info_header.v4.gammaRed));
        READ_FROM_BMP(&(bm.info_header.v4.gammaGreen));
        READ_FROM_BMP(&(bm.info_header.v4.gammaBlue));
    }

    // TODO: Do we need to worry about the V5 header?


    // Read the color table, if one exists

    // TODO: Do we also have to worry about compression here?
    //       See: https://stackoverflow.com/a/25072672
    if(bm.info_header.v1.colorsUsed > 0) {
        try {
            bm.color_table.reset(new RGBQuad[bm.info_header.v1.colorsUsed]);
        } catch(const std::bad_alloc& e) {
            WRITE_ERROR("Failed to allocate enough space for the bitmap's color table (",
                        bm.info_header.v1.colorsUsed * sizeof(RGBQuad),
                        " bytes required): ",
                        e.what());
            RETURN_ERROR(STATUS_BADALLOC);
        }

        for(auto i = 0U; i < bm.info_header.v1.colorsUsed; ++i) {
            READ_FROM_BMP(&(bm.color_table[i].blue));
            READ_FROM_BMP(&(bm.color_table[i].green));
            READ_FROM_BMP(&(bm.color_table[i].red));
            READ_FROM_BMP(&(bm.color_table[i].reserved));
        }
    }

    size_t depth = bm.info_header.v1.bitsPerPixel / 8;

    // Calculate how many bytes make up one line
    size_t orig_pitch = bm.info_header.v1.width * bm.info_header.v1.bitsPerPixel;
    size_t new_pitch = bm.info_header.v1.width * depth; // This is how many we _want_ each line to take up.

    // Allocate space for our new image data
    try {
        bm.data.reset(new unsigned char[new_pitch * bm.info_header.v1.height]);
    } catch(const std::bad_alloc& e) {
        WRITE_ERROR("Failed to allocate enough space for the bitmap's data (",
                    new_pitch * bm.info_header.v1.height, " bytes required): ",
                    e.what());

        RETURN_ERROR(STATUS_BADALLOC);
    }

    // Make sure we read where the file tells us the offset is, and not just
    //  assume that the data starts after the header (it doesn't always do that)
    if(auto pos = stream.tellg(); pos != bm.file_header.bitmapOffset) {
        WRITE_WARN("Did not read expected number of bytes from the header! We "
                   "expected to be at the bitmap offset (",
                   bm.file_header.bitmapOffset, ") but we are instead at ",
                   stream.tellg(), ". Attempting to seek to the expected offset.");

        stream.seekg(bm.file_header.bitmapOffset, stream.beg);
        WRITE_DEBUG("Current position after seek: ", stream.tellg());
    }

    // Read the pixel data from the stream next
    READ_FROM_BMP2(bm.data.get(), bm.info_header.v1.sizeOfBitmap);

    if(orig_pitch % 4 != 0) {
        WRITE_WARN("BitMap Width is not a multiple of 4! May contain up to ",
                   (orig_pitch / 4), " padding bytes.");
    }

    // Only flip the bytes for 24-bit images
    if(depth == 3) {
        WRITE_DEBUG("Swapping BGR to RGB before writing the BitMap.");
        // Swap B and R every 3 pixels since BitMap expects pixels in BGR rather
        //   than RGB format
        for(size_t i = 2; i < bm.info_header.v1.sizeOfBitmap; i += depth) {
            std::swap(bm.data[i], bm.data[i - 2]);
        }
    }

    //----------------
    // Flip the entire image, because BitMap is a weird format.
    //----------------
    auto res = flipImage(bm.data.get(), bm.data.get(), new_pitch,
                         bm.info_header.v1.height);
    RETURN_IF_ERROR(res);

    WRITE_DEBUG("Successfully loaded ", bm);

    return std::ref(bm);

#undef READ_FROM_BMP
#undef READ_FROM_BMP2
}

auto HMDT::readBMP2(std::filesystem::path& path) noexcept -> Maybe<BitMap2> {
    BitMap2 bm;

    auto res = readBMP(path, bm);
    RETURN_IF_ERROR(res);

    return bm;
}

auto HMDT::writeBMP(const std::filesystem::path& path,
                    std::shared_ptr<const BitMap2> bmp) noexcept
    -> MaybeVoid
{
    if(bmp == nullptr) {
       RETURN_ERROR(STATUS_PARAM_CANNOT_BE_NULL);
    }

    auto res = writeBMP(path, *bmp);
    RETURN_IF_ERROR(res);

    return res;
}

auto HMDT::writeBMP(const std::filesystem::path& path, const BitMap2& bmp) noexcept
    -> MaybeVoid
{
    std::ofstream file(path, std::ios::out | std::ios::binary);

    if(!file) {
        WRITE_ERROR("Failed to open output file ", path);
        RETURN_ERROR(std::error_code(errno, std::generic_category()));
    }

    WRITE_DEBUG(bmp);

    // Helper macro to make the following code easier to read
#define WRITE_BMP_VALUE(MEMBER) \
    file.write(reinterpret_cast<const char*>(&(MEMBER)), \
               sizeof(MEMBER))

    WRITE_BMP_VALUE(bmp.file_header.filetype);
    WRITE_BMP_VALUE(bmp.file_header.fileSize);
    WRITE_BMP_VALUE(bmp.file_header.reserved1);
    WRITE_BMP_VALUE(bmp.file_header.reserved2);
    WRITE_BMP_VALUE(bmp.file_header.bitmapOffset);
    WRITE_BMP_VALUE(bmp.info_header.v1.headerSize);
    WRITE_BMP_VALUE(bmp.info_header.v1.width);
    WRITE_BMP_VALUE(bmp.info_header.v1.height);
    WRITE_BMP_VALUE(bmp.info_header.v1.bitPlanes);
    WRITE_BMP_VALUE(bmp.info_header.v1.bitsPerPixel);
    WRITE_BMP_VALUE(bmp.info_header.v1.compression);
    WRITE_BMP_VALUE(bmp.info_header.v1.sizeOfBitmap);
    WRITE_BMP_VALUE(bmp.info_header.v1.horzResolution);
    WRITE_BMP_VALUE(bmp.info_header.v1.vertResolution);
    WRITE_BMP_VALUE(bmp.info_header.v1.colorsUsed);
    WRITE_BMP_VALUE(bmp.info_header.v1.colorImportant);

    // Write the V4 header
    if(bmp.info_header.v1.headerSize >= V4_INFO_HEADER_LENGTH) {
        WRITE_BMP_VALUE(bmp.info_header.v4.redMask);
        WRITE_BMP_VALUE(bmp.info_header.v4.greenMask);
        WRITE_BMP_VALUE(bmp.info_header.v4.blueMask);
        WRITE_BMP_VALUE(bmp.info_header.v4.alphaMask);
        WRITE_BMP_VALUE(bmp.info_header.v4.CSType);

        WRITE_BMP_VALUE(bmp.info_header.v4.redX);
        WRITE_BMP_VALUE(bmp.info_header.v4.redY);
        WRITE_BMP_VALUE(bmp.info_header.v4.redZ);
        WRITE_BMP_VALUE(bmp.info_header.v4.greenX);
        WRITE_BMP_VALUE(bmp.info_header.v4.greenY);
        WRITE_BMP_VALUE(bmp.info_header.v4.greenZ);
        WRITE_BMP_VALUE(bmp.info_header.v4.blueX);
        WRITE_BMP_VALUE(bmp.info_header.v4.blueY);
        WRITE_BMP_VALUE(bmp.info_header.v4.blueZ);

        WRITE_BMP_VALUE(bmp.info_header.v4.gammaRed);
        WRITE_BMP_VALUE(bmp.info_header.v4.gammaGreen);
        WRITE_BMP_VALUE(bmp.info_header.v4.gammaBlue);
    }

    // Write the V5 header
    if(bmp.info_header.v1.headerSize >= V5_INFO_HEADER_LENGTH) {
        WRITE_BMP_VALUE(bmp.info_header.v5.profileData);
        WRITE_BMP_VALUE(bmp.info_header.v5.profileSize);
        WRITE_BMP_VALUE(bmp.info_header.v5.reserved);
    }

    if(bmp.color_table != nullptr) {
        WRITE_DEBUG("Writing color table with ", bmp.info_header.v1.colorsUsed,
                    " values.");
        for(auto i = 0U; i < bmp.info_header.v1.colorsUsed; ++i) {
            WRITE_BMP_VALUE(bmp.color_table[i].blue);
            WRITE_BMP_VALUE(bmp.color_table[i].green);
            WRITE_BMP_VALUE(bmp.color_table[i].red);
            WRITE_BMP_VALUE(bmp.color_table[i].reserved);
        }
    }

    // Verify that the offset matches where in the file we are
    if(auto stream_loc = file.tellp();
            stream_loc != bmp.file_header.bitmapOffset)
    {
        WRITE_ERROR("Validation error! Calculated bitmapOffset of ",
                    bmp.file_header.bitmapOffset, " does not match number of "
                    "bytes we've written so far of ", stream_loc);
        RETURN_ERROR(STATUS_BITMAP_OFFSET_VALIDATION_ERROR);
    }

    //----------------
    // Flip the entire image, because BitMap is a weird format.
    //----------------

    // This is how much space a single line takes up
    size_t depth = bmp.info_header.v1.bitsPerPixel / 8;
    size_t pitch = bmp.info_header.v1.width * depth;

    WRITE_DEBUG("Flipping entire image before we write it. depth=", depth, ", pitch=", pitch);
    std::unique_ptr<unsigned char[]> output;
    try {
        output.reset(new unsigned char[bmp.info_header.v1.sizeOfBitmap]{ 0 });
    } catch(std::bad_alloc e) {
        WRITE_ERROR("Failed to allocate enough space for flipped output data: ", e.what());
        RETURN_ERROR(STATUS_BADALLOC);
    }

    auto res = flipImage(output.get(), bmp.data.get(), pitch, bmp.info_header.v1.height);
    RETURN_IF_ERROR(res);

    // Only flip the bytes for 24-bit images
    if(depth == 3) {
        WRITE_DEBUG("Swapping RGB to BGR before writing the BitMap.");
        // Swap B and R every 3 pixels since BitMap expects pixels in BGR rather
        //   than RGB format
        for(size_t i = 2; i < bmp.info_header.v1.sizeOfBitmap; i += depth) {
            std::swap(output[i], output[i - 2]);
        }
    }

    {
        WRITE_DEBUG("Image flipped successfully, writing ",
                    bmp.info_header.v1.sizeOfBitmap, " bytes to file.");
        auto before = file.tellp();
        file.write(reinterpret_cast<const char*>(output.get()),
                   bmp.info_header.v1.sizeOfBitmap);
        WRITE_DEBUG("Wrote ", (file.tellp() - before), " bytes.");
    }

    // Close the file and verify if the write succeeded
    file.close();
    if(file.bad()) {
        WRITE_ERROR("badbit set on output file after closing it. Write failed!");
        RETURN_ERROR(std::error_code(errno, std::generic_category()));
    }

    return STATUS_SUCCESS;

#undef WRITE_BMP_VALUE
}

auto HMDT::writeBMP2(const std::filesystem::path& path, unsigned char* data,
                     uint32_t width, uint32_t height, uint16_t depth,
                     bool is_greyscale, BMPHeaderToUse hdr_version_to_use) noexcept
    -> MaybeVoid
{
    BitMap2 bmp{};

    // Make sure we release the grabbed 'data' pointer when we exit, as it is
    //   not ours to delete
    RUN_AT_SCOPE_END([&bmp]() {
        bmp.data.release();
    });

    auto num_pixels = width * height;

    bmp.file_header.filetype = BM_TYPE;
    bmp.file_header.fileSize = FILE_HEADER_LENGTH + num_pixels * depth;
    bmp.file_header.reserved1 = 0;
    bmp.file_header.reserved2 = 0;
    bmp.file_header.bitmapOffset = FILE_HEADER_LENGTH;

    auto info_header_size = 0;

    // Fill out the info header based on what version is requested
    //   Note: we populate it backwards to take advantage of switch-case
    //   fallthrough
    switch(hdr_version_to_use) {
        case BMPHeaderToUse::V5:
            WRITE_DEBUG("Build V5 header");
            info_header_size += (V5_INFO_HEADER_LENGTH - V4_INFO_HEADER_LENGTH);

            bmp.info_header.v5.profileData = 0;
            bmp.info_header.v5.profileSize = 0;
            bmp.info_header.v5.reserved = 0;

            [[fallthrough]];
        case BMPHeaderToUse::V4:
            WRITE_DEBUG("Build V4 header");
            info_header_size += (V4_INFO_HEADER_LENGTH - V1_INFO_HEADER_LENGTH);

            bmp.info_header.v4.redMask   = 0x00FF0000;
            bmp.info_header.v4.greenMask = 0x0000FF00;
            bmp.info_header.v4.blueMask  = 0x000000FF;
            bmp.info_header.v4.alphaMask = 0xFF000000;
            bmp.info_header.v4.CSType = LogicalColorSpace::CALIBRATED_RGB;

            // endpoints
            bmp.info_header.v4.redX = 0;
            bmp.info_header.v4.redY = 0;
            bmp.info_header.v4.redZ = 0;
            bmp.info_header.v4.greenX = 0;
            bmp.info_header.v4.greenY = 0;
            bmp.info_header.v4.greenZ = 0;
            bmp.info_header.v4.blueX = 0;
            bmp.info_header.v4.blueY = 0;
            bmp.info_header.v4.blueZ = 0;

            bmp.info_header.v4.gammaRed = 0;
            bmp.info_header.v4.gammaGreen = 0;
            bmp.info_header.v4.gammaBlue = 0;

            [[fallthrough]];
        case BMPHeaderToUse::V1:
            WRITE_DEBUG("Build V1 header");
            info_header_size += V1_INFO_HEADER_LENGTH;

            bmp.info_header.v1.headerSize = info_header_size;
            bmp.info_header.v1.width = static_cast<int>(width);
            bmp.info_header.v1.height = static_cast<int>(height);
            bmp.info_header.v1.bitPlanes = 1;
            bmp.info_header.v1.bitsPerPixel = depth * 8; // 8 bits per pixel
            bmp.info_header.v1.compression = 0; // For Win32 systems, this is BI_RGB
            bmp.info_header.v1.sizeOfBitmap = num_pixels * depth;
            bmp.info_header.v1.horzResolution = 0; // TODO: Do we need to set this?
            bmp.info_header.v1.vertResolution = 0; // TODO: Do we need to set this?
            bmp.info_header.v1.colorsUsed = 0;
            bmp.info_header.v1.colorImportant = 0;
    }

    // Now that we know the info header size, add it to the relevant file header
    //   fields
    bmp.file_header.fileSize += info_header_size;
    bmp.file_header.bitmapOffset += info_header_size;

    if(bmp.info_header.v1.bitsPerPixel <= 8) {
        switch(bmp.info_header.v1.bitsPerPixel) {
            case 8:
                bmp.info_header.v1.colorsUsed = 256;
                break;
            case 4:
                bmp.info_header.v1.colorsUsed = 16;
                break;
            case 1:
                bmp.info_header.v1.colorsUsed = 2;
                break;
            default:
                WRITE_ERROR("Invalid bitsPerPixel<=8! Must be 1, 4, or 8, not ",
                            bmp.info_header.v1.bitsPerPixel);
                RETURN_ERROR(STATUS_INVALID_BITS_PER_PIXEL);
        }

        // colorsImportant will always match colorsUsed
        bmp.info_header.v1.colorImportant = bmp.info_header.v1.colorsUsed;

        WRITE_DEBUG("Generating color table with ", bmp.info_header.v1.colorsUsed,
                    " values.");

        auto res = createColorTable(bmp, is_greyscale);
        RETURN_IF_ERROR(res);
    }

    bmp.data.reset(data);

    auto res = writeBMP(path, bmp);
    RETURN_IF_ERROR(res);

    return STATUS_SUCCESS;
}


/**
 * @brief Generates a color table for the given bitmap
 *
 * @param bmp
 * @param is_greyscale
 *
 * @return 
 */
auto HMDT::createColorTable(BitMap2& bmp, bool is_greyscale) -> MaybeVoid {
    WRITE_DEBUG("Old File Header values:"
                " fileSize=", bmp.file_header.fileSize,
                ", bitmapOffset=", bmp.file_header.bitmapOffset,
                ", sizeOfBitmap=", bmp.info_header.v1.sizeOfBitmap);

    // Make sure we first remove any extra data that we would have from an older
    //   color table
    if(bmp.color_table != nullptr) {
        WRITE_DEBUG("Old color table is present, removing that first...");

        auto old_color_table_size = sizeof(RGBQuad) * bmp.info_header.v1.colorsUsed;
        bmp.file_header.fileSize -= old_color_table_size;
        bmp.file_header.bitmapOffset -= old_color_table_size;
    }

    // Increase all of the sizes and offsets
    auto color_table_size = sizeof(RGBQuad) * bmp.info_header.v1.colorsUsed;

    WRITE_DEBUG("New color table size = ", color_table_size, ". Shifting all "
                "values down to account for it.");

    bmp.file_header.fileSize += color_table_size;
    bmp.file_header.bitmapOffset += color_table_size;

    WRITE_DEBUG("Updated File Header values:"
                " fileSize=", bmp.file_header.fileSize,
                ", bitmapOffset=", bmp.file_header.bitmapOffset,
                ", sizeOfBitmap=", bmp.info_header.v1.sizeOfBitmap);

    bmp.color_table.reset(new RGBQuad[bmp.info_header.v1.colorsUsed]);

    // No need for a default block here, as we already checked that condition
    //   up above
    switch(bmp.info_header.v1.bitsPerPixel) {
        case 8:
        case 4:
            if(is_greyscale) {
                for(auto i = 0U; i < bmp.info_header.v1.colorsUsed; ++i) {
                    uint8_t c = i * (0x100 / bmp.info_header.v1.colorsUsed);
                    bmp.color_table[i] = { { c, c, c, 0x00 } };
                }
            } else {
                // TODO: Will we need to even support this case?
                RETURN_ERROR(STATUS_NOT_IMPLEMENTED);
            }
            break;
        case 1:
            bmp.color_table[0] = { { 0xFF, 0xFF, 0xFF, 0x00 } };
            bmp.color_table[1] = { { 0x00, 0x00, 0x00, 0x00 } };
            break;
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Outputs a BitMap to an ostream
 *
 * @param stream The output stream to write to.
 * @param bm The BitMap to write.
 *
 * @return The given stream after writing the BitMap to it.
 */
std::ostream& HMDT::operator<<(std::ostream& stream, const HMDT::BitMap2& bm) {
    stream << "BitMap = {" << std::endl
           << "    Header = {" << std::endl
           << "        filetype = " << bm.file_header.filetype << std::endl
           << "        fileSize = " << bm.file_header.fileSize << std::endl
           << "        reserved1 = " << bm.file_header.reserved1 << std::endl
           << "        reserved2 = " << bm.file_header.reserved2 << std::endl
           << "        bitmapOffset = " << bm.file_header.bitmapOffset << std::endl
           << "    }" << std::endl
           << "    headerSize = " << bm.info_header.v1.headerSize << std::endl
           << "    width = " << bm.info_header.v1.width << std::endl
           << "    height = " << bm.info_header.v1.height << std::endl
           << "    bitPlanes = " << bm.info_header.v1.bitPlanes << std::endl
           << "    bitsPerPixel = " << bm.info_header.v1.bitsPerPixel << std::endl
           << "    compression = " << bm.info_header.v1.compression << std::endl
           << "    sizeOfBitmap = " << bm.info_header.v1.sizeOfBitmap << std::endl
           << "    horzResolution = " << bm.info_header.v1.horzResolution << std::endl
           << "    vertResolution = " << bm.info_header.v1.vertResolution << std::endl
           << "    colorsUsed = " << bm.info_header.v1.colorsUsed << std::endl
           << "    colorImportant = " << bm.info_header.v1.colorImportant << std::endl;

    // Optionally print out the v4 header if this bitmap has one
    if(bm.info_header.v1.headerSize >= 108) {
        stream << "    redMask = " << bm.info_header.v4.redMask << std::endl;
        stream << "    greenMask = " << bm.info_header.v4.greenMask << std::endl;
        stream << "    blueMask = " << bm.info_header.v4.blueMask << std::endl;
        stream << "    alphaMask = " << bm.info_header.v4.alphaMask << std::endl;
        stream << "    CSType = " << static_cast<uint32_t>(bm.info_header.v4.CSType) << std::endl;

        // endpoints
        stream << "    endpoints = {" << std::endl
               << "        red = { " << bm.info_header.v4.redX << " "
                                     << bm.info_header.v4.redY << " "
                                     << bm.info_header.v4.redZ << "}" << std::endl
               << "        green = { " << bm.info_header.v4.greenX << " "
                                       << bm.info_header.v4.greenY << " "
                                       << bm.info_header.v4.greenZ << "}" << std::endl
               << "        blue = { " << bm.info_header.v4.blueX << " "
                                      << bm.info_header.v4.blueY << " "
                                      << bm.info_header.v4.blueZ << "}" << std::endl
               << "    }" << std::endl;

        stream << "    gammaRed = " << bm.info_header.v4.gammaRed << std::endl;
        stream << "    gammaGreen = " << bm.info_header.v4.gammaGreen << std::endl;
        stream << "    gammaBlue = " << bm.info_header.v4.gammaBlue << std::endl;
    }

    // Optionally print out the v5 header if this bitmap has one
    if(bm.info_header.v1.headerSize >= 124) {
        stream << "    profileData = " << bm.info_header.v5.profileData << std::endl;
        stream << "    profileSize = " << bm.info_header.v5.profileSize << std::endl;
        stream << "    reserved = " << bm.info_header.v5.reserved << std::endl;
    }

    stream << "    colorTable = { ";
    if(bm.color_table != nullptr) {
        for(auto i = 0U; i < bm.info_header.v1.colorsUsed; ++i) {
            stream << "0x" << std::hex << (static_cast<uint32_t>(bm.color_table[i].blue) << 24 |
                                           static_cast<uint32_t>(bm.color_table[i].green) << 16 |
                                           static_cast<uint32_t>(bm.color_table[i].red) << 8 |
                                           static_cast<uint32_t>(bm.color_table[i].reserved))
                           << std::dec << ", ";
        }
    } else {
        stream << "<NULL>";
    }
    stream << " }" << std::endl
           << "    data = { ... }" << std::endl
           << "}";

    return stream;
}

