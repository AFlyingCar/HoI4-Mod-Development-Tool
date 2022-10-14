/**
 * @file BitMap.h
 *
 * @brief Defines all data about BitMap reading and writing
 */

#ifndef _BITMAP_H_
# define _BITMAP_H_

# include <ostream> // std::ostream
# include <filesystem> // std::filesystem::path

# include "Maybe.h"

namespace HMDT {
    //! The length of the FileHeader
    constexpr uint32_t FILE_HEADER_LENGTH = 14;

    //! The length of the V1 InfoHeader
    constexpr uint32_t V1_INFO_HEADER_LENGTH = 40;

    //! The length of the V4 InfoHeader (108 bytes)
    constexpr uint32_t V4_INFO_HEADER_LENGTH = V1_INFO_HEADER_LENGTH + 68;

    //! The length of the V5 InfoHeader (124 bytes)
    constexpr uint32_t V5_INFO_HEADER_LENGTH = V4_INFO_HEADER_LENGTH + 16;

    /**
     * @brief Defines the file header section of a .BMP file.
     */
    struct BitMapFileHeader {
        uint16_t filetype;     //! The filetype
        uint32_t fileSize;     //! The size of the file.
        uint16_t reserved1;    //! RESERVED
        uint16_t reserved2;    //! RESERVED
        uint32_t bitmapOffset; //! How far into the file the bitmap starts.
    };

    /**
     * @brief Defines the info section of a .BMP file.
     */
    struct BitMapInfoHeader {
        unsigned int headerSize;     //! The size of the header
        int width;                   //! The width of the file
        int height;                  //! The height of the file
        unsigned short bitPlanes;    //! IGNORED
        unsigned short bitsPerPixel; //! The number of bits making up each pixel
        unsigned int compression;    //! IGNORED
        unsigned int sizeOfBitmap;   //! Size of the image data
        unsigned int horzResolution; //! IGNORED
        unsigned int vertResolution; //! IGNORED
        unsigned int colorsUsed;     //! IGNORED
        unsigned int colorImportant; //! IGNORED
    };

    /**
     * @brief Specifies the type of color space.
     * @details For more information, see the Microsoft documentation at:
     *          https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-wmf/eb4bbd50-b3ce-4917-895c-be31f214797f
     */
    enum class LogicalColorSpace: uint32_t {
        CALIBRATED_RGB = 0,
        SRGB = 0x73524742,
        WINDOWS_COLOR_SPACE = 0x57696E20
    };

    /**
     * @brief Defines the V4 info section of a .BMP file
     */
    struct BitMapInfoHeaderV4 {
        BitMapInfoHeader v1; //! The V1 header information

        uint32_t redMask;
        uint32_t greenMask;
        uint32_t blueMask;
        uint32_t alphaMask;
        LogicalColorSpace CSType;

        // endpoints
        uint32_t redX;
        uint32_t redY;
        uint32_t redZ;
        uint32_t greenX;
        uint32_t greenY;
        uint32_t greenZ;
        uint32_t blueX;
        uint32_t blueY;
        uint32_t blueZ;

        uint32_t gammaRed;
        uint32_t gammaGreen;
        uint32_t gammaBlue;
    };

    struct BitMapInfoHeaderV5 {
        BitMapInfoHeaderV4 v4; //! The V4 header information

        uint32_t profileData;
        uint32_t profileSize;
        uint32_t reserved;
    };

    /**
     * @brief Defines the RGBQUAD struct used in the .BMP file's color table.
     */
    union RGBQuad {
        struct {
            uint8_t blue;
            uint8_t green;
            uint8_t red;
            uint8_t reserved; // Alpha?
        };

        uint32_t rgb_quad;
    };

    /**
     * @brief A simple representation of a Bit Map image.
     */
    struct [[deprecated]] BitMap {
        BitMapFileHeader file_header; //! The file header of the BitMap
        BitMapInfoHeader info_header; //! The info header of the BitMap

        unsigned char* data;          //! The image data
    };

    /**
     * @brief A representation of a BitMap image
     * @details The info_header  will be one of v1, v4, or v5 depending on the
     *          value of BitMapInfoHeader::headerSize. V1 is guaranteed to
     *          always be valid.
     */
    struct BitMap2 {
        BitMapFileHeader file_header; //! The file header of the BitMap

        /**
         * @brief This union will hold each different possible header.
         * @details Every header _must_ be listed in the order in which they
         *          inherit from each other, so that the memory layout is
         *          exactly the same regardless of if we access the smaller
         *          headers directly or via the larger headers.
         */
        union {
            BitMapInfoHeader v1;      //! V1 header
            BitMapInfoHeaderV4 v4;    //! V4 header
            BitMapInfoHeaderV5 v5;    //! V5 header
        } info_header;                //! The info header of the BitMap

        std::unique_ptr<RGBQuad[]> color_table; //! The color table of the BitMap. Can be nullptr
        std::unique_ptr<unsigned char[]> data;  //! The image data
    };

    [[deprecated]] BitMap* readBMP(const std::filesystem::path&, BitMap*);
    [[deprecated]] BitMap* readBMP(std::istream&, BitMap*);
    [[deprecated]] BitMap* readBMP(const std::filesystem::path&);

    [[deprecated]] void writeBMP(const std::filesystem::path&, const BitMap*);
    [[deprecated]] void writeBMP(const std::filesystem::path&, unsigned char*,
                                 uint32_t, uint32_t, uint16_t = 3);

    std::ostream& operator<<(std::ostream&, const HMDT::BitMap&);


    /**
     * @brief An enum listing all supported BMP info headers
     */
    enum class BMPHeaderToUse { V1, V4, V5 };

    /**
     * @brief A structure abstracting a color table
     */
    struct ColorTable {
        uint32_t num_colors;
        std::unique_ptr<RGBQuad[]> color_table;
    };

    MaybeRef<BitMap2> readBMP(const std::filesystem::path&,
                              std::shared_ptr<BitMap2>) noexcept;
    MaybeRef<BitMap2> readBMP(std::istream&, std::shared_ptr<BitMap2>) noexcept;

    MaybeRef<BitMap2> readBMP(const std::filesystem::path&, BitMap2&) noexcept;
    MaybeRef<BitMap2> readBMP(std::istream&, BitMap2&) noexcept;
    Maybe<BitMap2> readBMP2(std::filesystem::path&) noexcept;

    MaybeVoid writeBMP(const std::filesystem::path&,
                       std::shared_ptr<const BitMap2>) noexcept;
    MaybeVoid writeBMP(const std::filesystem::path&, const BitMap2&) noexcept;
    MaybeVoid writeBMP2(const std::filesystem::path&, unsigned char*,
                        uint32_t, uint32_t, uint16_t = 3, bool = false,
                        BMPHeaderToUse = BMPHeaderToUse::V4,
                        MonadOptional<ColorTable> = std::nullopt) noexcept;

    MaybeVoid createColorTable(BitMap2&, ColorTable&&, bool = false);
    MaybeVoid createColorTable(BitMap2&, bool = false);

    MaybeVoid convertBitMapTo8BPPGreyscale(BitMap2&) noexcept;

    std::ostream& operator<<(std::ostream&, const HMDT::BitMap2&);
}

#endif

