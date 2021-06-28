/**
 * @file BitMap.h
 *
 * @brief Defines all data about BitMap reading and writing
 */

#ifndef _BITMAP_H_
# define _BITMAP_H_

# include <ostream> // std::ostream
# include <filesystem> // std::filesystem::path

namespace MapNormalizer {
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
        unsigned int headerSize;     //! The size of the header (always 40)
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
     * @brief A simple representation of a Bit Map image.
     */
    struct BitMap {
        BitMapFileHeader file_header; //! The file header of the BitMap
        BitMapInfoHeader info_header; //! The info header of the BitMap
        unsigned char* data;          //! The image data
    };

    BitMap* readBMP(const std::filesystem::path&, BitMap*);
    BitMap* readBMP(const std::filesystem::path&);

    void writeBMP(const std::filesystem::path&, const BitMap*);
    void writeBMP(const std::filesystem::path&, unsigned char*, uint32_t, uint32_t);

    std::ostream& operator<<(std::ostream&, const MapNormalizer::BitMap&);
}

#endif

