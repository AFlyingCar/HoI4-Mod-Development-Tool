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

    uint64_t xyToIndex(const BitMap*, uint32_t, uint32_t);
    uint64_t xyToIndex(uint32_t, uint32_t, uint32_t);
    Color getColorAt(const BitMap*, uint32_t, uint32_t, uint32_t = 3);
    Pixel getAsPixel(const BitMap*, uint32_t, uint32_t, uint32_t = 3);

    bool isInImage(const BitMap*, uint32_t, uint32_t);

    bool isShapeTooLarge(uint32_t, uint32_t, const BitMap*);
    std::pair<uint32_t, uint32_t> calcDims(const BoundingBox&);
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

    /**
     * @brief Writes a value directly to an ostream
     *
     * @tparam T The type of data to write
     * @param stream The stream to write to
     * @param data The data to write
     */
    template<typename T>
    void writeData(std::ostream& stream, const T& data) {
        stream.write(reinterpret_cast<const char*>(&data), sizeof(T));
    }

    /**
     * @brief Writes multiple values directly to an ostream
     *
     * @tparam Ts The types of data to write
     * @param stream The stream to write to
     * @param datas The data values to write
     */
    template<typename... Ts>
    void writeData(std::ostream& stream, const Ts&... datas) {
        (writeData(stream, datas), ...);
    }

    /**
     * @brief Converts a string to a type
     *
     * @tparam T The type to convert to
     * @param s The string to convert
     *
     * @return The converted type
     */
    template<typename T>
    std::optional<T> fromString(const std::string& s) noexcept {
        if constexpr(std::is_same_v<T, std::string>) {
            return s;
        } else if constexpr(std::is_same_v<T, bool>) {
            if(s == "true" || s == "1") {
                return true;
            } else if(s == "false" || s == "0") {
                return false;
            } else {
                return std::nullopt;
            }
        } else if constexpr(std::is_integral_v<T>) {
            try {
                return static_cast<T>(std::stoi(s));
            } catch(const std::invalid_argument&) {
                return std::nullopt; // TODO: print error
            }
        } else if constexpr(std::is_floating_point_v<T>) {
            try {
                return static_cast<T>(std::stof(s));
            } catch(const std::invalid_argument&) {
                return std::nullopt;
            }
        } else if constexpr(std::is_same_v<T, ProvinceType>) {
            if(s == "land") {
                return ProvinceType::LAND;
            } else if(s == "sea") {
                return ProvinceType::SEA;
            } else if(s == "lake") {
                return ProvinceType::LAKE;
            } else {
                return ProvinceType::UNKNOWN;
            }
        } else {
            static_assert("Unsupported type!");

            return std::nullopt;
        }
    }

    /**
     * @brief Parses a type out of a string, with a known delimiter
     *
     * @tparam T The type to parse out
     * @param stream The stream to parse from
     * @param result The location to place the parsed value
     * @param delim The delimiter to use when parsing out the string
     *
     * @return True if the value was successfully parsed, false otherwise
     */
    template<typename T>
    bool parseValue(std::istream& stream, T& result, char delim = ' ') noexcept {
        if(std::string s; std::getline(stream, s, delim)) {
            if(auto opt_result = fromString<T>(s); opt_result) {
                result = *opt_result;
                return true;
            }

            return false;
        }

        if(auto opt_result = fromString<T>(""); opt_result) {
            result = *opt_result;
            return true;
        }

        return false;
    }

    /**
     * @brief Parses multiple types out of a string, with a known delimiter
     *
     * @tparam Delim The delimiter to use when parsing out the string
     * @tparam Ts The types to parse out in the order they should be in
     * @param stream The stream to parse from
     * @param results The locations to place the parsed values
     *
     * @return True if the value was successfully parsed, false otherwise
     */
    template<char Delim = ' ', typename... Ts>
    bool parseValues(std::istream& stream, Ts&... results) noexcept {
        return (parseValue(stream, results, Delim) && ...);
    }

    ProvinceList createProvincesFromShapeList(const PolygonList&);
}

#endif

