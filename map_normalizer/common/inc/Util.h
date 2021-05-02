/**
 * @file Util.h
 *
 * @brief Defines various utility functions.
 */

#ifndef UTIL_H
# define UTIL_H

# include <cstdint>
# include <algorithm>
# include <istream>

# include "Types.h"

namespace MapNormalizer {
    // Forward declare this, as we don't need to include the whole file yet.
    struct BitMap;

    std::uint32_t indexOfLSB(std::uint32_t);
    std::uint32_t swapBytes(std::uint32_t);
    std::uint32_t colorToRGB(const Color&);
    Color RGBToColor(std::uint32_t);
    bool doColorsMatch(const Color&, const Color&);

    uint64_t xyToIndex(BitMap*, uint32_t, uint32_t);
    uint64_t xyToIndex(uint32_t, uint32_t, uint32_t);
    Color getColorAt(const BitMap*, uint32_t, uint32_t, uint32_t = 3);
    Pixel getAsPixel(const BitMap*, uint32_t, uint32_t, uint32_t = 3);

    bool isInImage(BitMap*, uint32_t, uint32_t);

    bool isShapeTooLarge(uint32_t, uint32_t, BitMap*);
    std::pair<uint32_t, uint32_t> calcShapeDims(const Polygon&);

    void ltrim(std::string&);
    void rtrim(std::string&);
    void trim(std::string&);

    template<typename T>
    T clamp(T val, T min, T max) {
        return std::min(std::max(val, min), max);
    }

    void writeColorTo(unsigned char*, uint32_t, uint32_t, uint32_t, Color);

    // Taken from https://en.cppreference.com/w/cpp/utility/variant/visit
    template<typename... Ts>
    struct overloaded: Ts... {
        using Ts::operator()...;
    };

    template<typename... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

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
    bool safeRead(T* destination, std::istream& stream) {
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
    bool safeRead(T* destination, size_t size, std::istream& stream) {
        if(!stream.eof() && stream.good()) {
            stream.read(reinterpret_cast<char*>(destination), size);
            return true;
        } else {
            return false;
        }
    }

    /**
     * @brief Safely reads from a stream if and only if the stream has not
     *        reached eof and is still good.
     *
     * @tparam Ts All types of data to read.
     * @param stream The stream to read from.
     * @param destinations Each pointer to data locations to read into.
     *
     * @return True if the reads were successful, false otherwise.
     */
    template<typename... Ts>
    bool safeRead(std::istream& stream, Ts*... destinations) {
        return (safeRead(destinations, stream) && ...);
    }

    template<typename T>
    void writeData(std::ostream& stream, const T& data) {
        stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
    }

    template<typename... Ts>
    void writeData(std::ostream& stream, const Ts&... datas) {
        (writeData(stream, datas), ...);
    }
}

#endif

