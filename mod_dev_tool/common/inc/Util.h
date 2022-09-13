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
# include <memory>
# include <filesystem>
# include <functional>
# include <optional>
# include <thread>
# include <future>

# include "Types.h"
# include "Logger.h"
# include "PreprocessorUtils.h"
# include "TypeTraits.h"
# include "Maybe.h"
# include "StatusCodes.h"

namespace HMDT {
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
    Color getColorAt(const Dimensions&, const uint8_t*,
                     uint32_t, uint32_t, uint32_t = 3);
    Pixel getAsPixel(const BitMap*, uint32_t, uint32_t, uint32_t = 3);

    bool isInImage(const Dimensions&, uint32_t, uint32_t);

    bool isShapeTooLarge(uint32_t, uint32_t, const BitMap*);
    std::pair<uint32_t, uint32_t> calcDims(const BoundingBox&);
    std::pair<uint32_t, uint32_t> calcShapeDims(const Polygon&);

    void ltrim(std::string&);
    void rtrim(std::string&);
    void trim(std::string&);

    std::string ltrim(const std::string&);
    std::string rtrim(const std::string&);
    std::string trim(const std::string&);

    std::string& toLower(std::string&);
    std::string toLower(const std::string&);

    std::string& toUpper(std::string&);
    std::string toUpper(const std::string&);

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
            return stream.gcount() == sizeof(T);
        } else {
            WRITE_ERROR("Failed to safely read ", sizeof(T),
                        " bytes. EOF=", stream.eof(), ", good=", stream.good());
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
            return stream.gcount() == size;
        } else {
            WRITE_ERROR("Failed to safely read ", size,
                        " bytes. EOF=", stream.eof(), ", good=", stream.good());
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
     * @brief Safely reads from stream if and only if the stream has not reached eof
     *        and is still good.
     *
     * @tparam T The type of data to read.
     * @param destination A pointer to the data location to read into.
     * @param stream The stream to read from.
     * @return True if the read was successful, false otherwise.
     */
    template<typename T>
    MaybeVoid safeRead2(T* destination, std::istream& stream) {
        if(!stream.eof() && stream.good()) {
            stream.read(reinterpret_cast<char*>(destination), sizeof(T));
            RETURN_ERROR_IF(stream.gcount() != sizeof(T),
                            STATUS_READ_TOO_FEW_BYTES);
        } else {
            WRITE_ERROR("Failed to safely read ", sizeof(T),
                        " bytes. EOF=", stream.eof(), ", good=", stream.good());
            RETURN_ERROR(STATUS_CANNOT_READ_FROM_STREAM);
        }

        return STATUS_SUCCESS;
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
    MaybeVoid safeRead2(T* destination, size_t size, std::istream& stream) {
        if(!stream.eof() && stream.good()) {
            stream.read(reinterpret_cast<char*>(destination), size);
            RETURN_ERROR_IF(stream.gcount() != size, STATUS_READ_TOO_FEW_BYTES);
        } else {
            WRITE_ERROR("Failed to safely read ", size,
                        " bytes. EOF=", stream.eof(), ", good=", stream.good());
            RETURN_ERROR(STATUS_CANNOT_READ_FROM_STREAM);
        }

        return STATUS_SUCCESS;
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
    template<typename T1, typename... Ts>
    MaybeVoid safeRead2(std::istream& stream, T1* dest1, Ts*... destinations) {
        auto res = safeRead2(dest1, stream);
        RETURN_IF_ERROR(res);

        // Only bother recursing again if there still more destinations left.
        if constexpr(sizeof...(Ts) > 0) {
            auto res = safeRead2<Ts...>(stream, destinations...);
            RETURN_IF_ERROR(res);
        }

        return STATUS_SUCCESS;
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
            if constexpr(std::is_unsigned_v<T>) {
                try {
                    return static_cast<T>(std::stoul(s));
                } catch(const std::invalid_argument& e) {
                    WRITE_ERROR("Received invalid argument, cannot convert '", s,
                                "' to int: what()=", e.what());
                    return std::nullopt;
                } catch(const std::out_of_range& e) {
                    WRITE_ERROR("Received out-of-range argument, cannot convert '",
                                s, "' to int: what()=", e.what());
                    return std::nullopt;
                }
            } else {
                try {
                    return static_cast<T>(std::stoi(s));
                } catch(const std::invalid_argument& e) {
                    WRITE_ERROR("Received invalid argument, cannot convert '", s,
                                "' to int: what()=", e.what());
                    return std::nullopt;
                } catch(const std::out_of_range& e) {
                    WRITE_ERROR("Received out-of-range argument, cannot convert '",
                                s, "' to int: what()=", e.what());
                    return std::nullopt;
                }
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
     * @details If the value is missing from the string, the stream is left in
     *          the same position as it started
     *
     * @tparam T The type to parse out
     * @param stream The stream to parse from
     * @param result The location to place the parsed value
     * @param delim The delimiter to use when parsing out the string
     * @param skip_missing Whether the value should be skipped if it's missing
     *
     * @return True if the value was successfully parsed, false otherwise
     */
    template<typename T>
    bool parseValue(std::istream& stream, T& result, char delim = ' ',
                    bool skip_missing = false) noexcept
    {
        // Save the stream's position first so we can rollback if needed
        std::streampos orig_pos = stream.tellg();
        if(std::string s; std::getline(stream, s, delim)) {
            if(auto opt_result = fromString<T>(s); opt_result) {
                result = *opt_result;
                return true;
            }

            stream.seekg(orig_pos);

            return skip_missing;
        }

        // If we failed to get a line from the stream, see if we can parse an
        //   empty string instead. This is to take care of the edge-case of
        //   there being an empty string after the last delimiter which could
        //   possibly still get parsed
        if(auto opt_result = fromString<T>(""); opt_result) {
            result = *opt_result;
            return true;
        }

        stream.seekg(orig_pos);

        return skip_missing;
    }

    /**
     * @brief No-op parser. Returns true
     *
     * @return true
     */
    template<char Delim = ' '>
    bool parseValuesSkipMissing(std::istream&) noexcept {
        return true;
    }

    /**
     * @brief Parses the next value out of a string, with a known delimiter. May
     *        skip the value if it is missing and skip_missing == true.
     * @details This function has optional boolean parameters after each
     *          location to output into. Usage is as follows:
     * @code{.cpp}
     * parseValuesSkipMissing(my_stream, &v1, &v2,
     *                                   &v3, true, // This value will be skipped if it is missing
     *                                   &v4);
     * @endcode
     *
     * @tparam Delim The delimiter to use when parsing out the string
     * @tparam T The next type to parse out of the stream
     * @tparam Ts The rest of the types to parse out of the stream
     *
     * @param stream The stream to parse from
     * @param result1 The next location to place the parsed value
     * @param skip_missing Whether this value should be skipped if it is missing
     *                     from the stream
     * @param results The rest of the locations to place the parsed values.
     *
     * @return True if the values were successfully parsed, false otherwise.
     */
    template<char Delim = ' ', typename T, typename... Ts>
    bool parseValuesSkipMissing(std::istream& stream, T* result1,
                                bool skip_missing, Ts... results) noexcept
    {
        return parseValue(stream, *result1, Delim, skip_missing) &&
               parseValuesSkipMissing<Delim>(stream, results...);
    }

    /**
     * @brief Parses the next value out of a string, with a known delimiter.
     * @details This function has optional boolean parameters after each
     *          location to output into. Usage is as follows:
     * @code{.cpp}
     * parseValuesSkipMissing(my_stream, &v1, &v2,
     *                                   &v3, true, // This value will be skipped if it is missing
     *                                   &v4);
     * @endcode
     *
     * @tparam Delim The delimiter to use when parsing out the string
     * @tparam T The next type to parse out of the stream
     * @tparam Ts The rest of the types to parse out of the stream
     *
     * @param stream The stream to parse from
     * @param result1 The next location to place the parsed value
     * @param results The rest of the locations to place the parsed values.
     *
     * @return True if the values were successfully parsed, false otherwise.
     */
    template<char Delim = ' ', typename T, typename... Ts>
    bool parseValuesSkipMissing(std::istream& stream, T* result1, Ts... results) noexcept
    {
        return parseValue(stream, *result1, Delim, false) &&
               parseValuesSkipMissing<Delim>(stream, results...);
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
    bool parseValues(std::istream& stream, Ts*... results) noexcept {
        return (parseValue(stream, *results, Delim, false) && ...);
    }

    ProvinceList createProvincesFromShapeList(const PolygonList&);

    std::filesystem::path getExecutablePath();

    /**
     * @brief Will split the given string, transform each value using the
     *        given unary function, and place each result into the output
     *        iterator 'out'.
     *
     * @tparam T The result type of the transform function.
     * @tparam C The output iterator type.
     *
     * @param str The string to split.
     * @param delim The delimiter to split the string at.
     * @param out The output iterator to place each transformed value.
     * @param func The function to use to transform each split value.
     */
    template<typename T, typename C>
    void splitAndTransform(const std::string& str, char delim, C out,
                           const std::function<T(const std::string&)>& func)
    {
        std::stringstream stream(str);
        std::string item;

        while(std::getline(stream, item, delim)) {
            if(item[0] == '\0') continue;
            *(out++) = func(item);
        }
    }

    /**
     * @brief Will split the given string, transform each value using the
     *        given unary function, and return a vector of all values.
     *
     * @tparam T The result type of the transform function.
     *
     * @param str The string to split.
     * @param delim The delimiter to split the string at.
     * @param func The function to use to transform each split value.
     *
     * @return A vector containing the transformed components of the split.
     */
    template<typename T>
    std::vector<T> splitAndTransform(const std::string& str, char delim,
                                     const std::function<T(const std::string&)>& func)
    {
        std::vector<T> result;
        splitAndTransform<T>(str, delim, std::back_inserter(result), func);
        return result;
    }

    template<typename InputIt, typename OutputIt, typename UnaryOperation>
    void parallelTransform(InputIt first, InputIt last, OutputIt d_first,
                           UnaryOperation unary_op)
    {
        auto thread_count = std::thread::hardware_concurrency();

        if(thread_count <= 1) {
            std::transform(first, last, d_first, unary_op);
        } else {
            using namespace std::chrono_literals;

            // The actual transformation function
            auto func = [&unary_op](InputIt first, InputIt last, OutputIt d_first)
            {
                for(; first != last; ++first) {
                    *d_first++ = unary_op(*first);
                }
            };

            std::vector<std::shared_future<void>> futures;

            auto length = std::distance(first, last);

            // If we have less data than cores, just spin up one core per value,
            //  and don't bother spinning up more cores for empty data
            if(length < thread_count) {
                thread_count = length;
            }

            auto it_step = length / thread_count;

            // start transforming each section of the input range in parallel
            for(auto i = 0; i < thread_count; ++i) {
                auto d_first2 = d_first;

                futures.push_back(std::async(std::launch::async, func,
                                             first, first + it_step, d_first2));

                // Advance all 3 iterators to the next part
                std::advance(first, it_step);
                std::advance(d_first, it_step);
            }

            // Keep checking each future until they have completed
            while(!futures.empty()) {
                for(auto it = futures.begin(); it != futures.end();) {
                    if(it->wait_for(1s) == std::future_status::ready) {
                        it = futures.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
    }

    /**
     * @brief Joins a range of values together into a string.
     *
     * @tparam Iter The type of iterator for traversing the range.
     *
     * @param begin The start of the range.
     * @param end The end of the range.
     * @param glue The string to use in between each value in the range.
     *
     * @return A string containing every value in the range, separated by 'glue'
     */
    template<typename Iter>
    std::string join(Iter begin, Iter end, const std::string& glue = "") {
        std::stringstream ss;
        for(Iter it = begin; it != end; ++it) {
            if(it != begin) ss << glue;
            ss << *it;
        }

        return ss.str();
    }

    /**
     * @brief Runs the given function when the current scope ends
     */
    struct RunAtScopeEnd {
        RunAtScopeEnd(const std::function<void()>& f): m_f(f) { }

        ~RunAtScopeEnd() {
            m_f();
        }

        std::function<void()> m_f;
    };

# define RUN_AT_SCOPE_END(...) \
    HMDT::RunAtScopeEnd HMDT_UNIQUE_NAME(___AT_SCOPE_END) ( __VA_ARGS__ )

    /**
     * @brief Alias type around std::reference_wrapper
     *
     * @tparam T The T to wrap a reference for
     */
    template<typename T>
    using Ref = std::reference_wrapper<T>;
}

#endif

