#ifndef _BITMAP_H_
#define _BITMAP_H_

#include <fstream> // std::ifstream
#include <string> // std::string

namespace MapNormalizer {
    struct BitMapHeader {
        uint16_t filetype;     //! The filetype
        uint32_t fileSize;     //! The size of the file.
        uint16_t reserved1;    //! RESERVED
        uint16_t reserved2;    //! RESERVED
        uint32_t bitmapOffset; //! How far into the file the bitmap starts.
    };

    /**
     * @brief A simple representation of a Bit Map image.
     */
    struct BitMap {
        // const char* filename;        //! The filename
        BitMapHeader header;         //! The header of the BitMap
        unsigned int headerSize;     //! The size of the header (always 40)
        int width;                   //! The width of the file
        int height;                  //! The height of the file
        unsigned short bitPlanes;    //! IGNORED
        unsigned short bitsPerPixel; //! IGNORED
        unsigned int compression;    //! IGNORED
        unsigned int sizeOfBitmap;   //! Size of the image data
        unsigned int horzResolution; //! IGNORED
        unsigned int vertResolution; //! IGNORED
        unsigned int colorsUsed;     //! IGNORED
        unsigned int colorImportant; //! IGNORED

        unsigned char* data; //! The image data
    };

    /**
     * @brief Safely reads from stream if and only if the stream has not reached eof
     *        and is still good.
     *
     * @tparam T The type of data to read.
     * @param destination A pointer to the data location to read into.
     * @param stream The stream to read from.
     * @return True if the read was successful, false otherwise.
     */
    template<typename T>
    bool safeRead(T* destination, std::ifstream& stream) {
        if(!stream.eof() && stream.good()) {
            stream.read(reinterpret_cast<char*>(destination), sizeof(T));
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief Safely reads from stream if and only if the stream has not reached eof
     *        and is still good.
     *
     * @tparam T The type of data to read.
     * @param destination A pointer to the data location to read into.
     * @param size The total number of bytes to read.
     * @param stream The stream to read from.
     * @return True if the read was successful, false otherwise.
     */
    template<typename T>
    bool safeRead(T* destination, size_t size, std::ifstream& stream) {
        if(!stream.eof() && stream.good()) {
            stream.read(reinterpret_cast<char*>(destination), size);
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief Reads a bitmap file.
     *
     * @param filename The filename to read from.
     * @return A pointer to a bitmap struct if the bitmap was successfully read,
     *         nullptr otherwise.
     */
    BitMap* readBMP(const std::string&);

    /**
     * @brief Writes a bitmap file
     *
     * @param filename The filename to write to
     * @param bmp The BitMap object to write.
     */
    void writeBMP(const std::string&, const BitMap*);

    /**
     * @brief Writes a bitmap file
     *
     * @param filename The filename to write to
     * @param data The color data to write
     * @param width The width of the bitmap
     * @param height The height of the bitmap
     */
    void writeBMP(const std::string&, unsigned char*, uint32_t, uint32_t);
}

#endif

