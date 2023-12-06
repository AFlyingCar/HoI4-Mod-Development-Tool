
#include "Util.h"

#include <cstdlib>
#include <cstdio>
#include <cstdlib>

#include "Constants.h"
#include "BitMap.h"

#ifdef _WIN32
# include "windows.h"
# ifndef PATH_MAX
#  define PATH_MAX FILENAME_MAX
# endif
#else
# include <unistd.h>
# include <linux/limits.h>
# include <execinfo.h>
# include <cxxabi.h>
#endif

// Helper replacement for __builtin_ctz if on MSVC
#ifdef MSC_VER
# include <intrin>

// https://stackoverflow.com/a/20468180
static std::uint32_t __inline __builtin_ctz(std::uint32_t value) {
    DWORD trailing_zero = 0;

    if(_BitScanForward(&trailing_zero, value)) return trailing_zero;
    return 0;
}

// https://stackoverflow.com/a/105339
static std::uint32_t __inline __builtin_bswap32(std::uint32_t value) {
    return _byteswap_ulong(value);
}

#endif

// https://stackoverflow.com/a/31393298
std::uint32_t HMDT::indexOfLSB(std::uint32_t value) {
    return __builtin_ctz(value);// + 1;
}

std::uint32_t HMDT::swapBytes(std::uint32_t value) {
    return __builtin_bswap32(value);
}

std::uint32_t HMDT::colorToRGB(const Color& color) {
    return ((color.g << 8) | (color.r << 16) | color.b) & COLOR_MASK;
}

HMDT::Color HMDT::RGBToColor(std::uint32_t rgb) {
    return Color{ static_cast<std::uint8_t>((rgb & RED_MASK) >> 16),
                  static_cast<std::uint8_t>((rgb & GREEN_MASK) >> 8),
                  static_cast<std::uint8_t>((rgb & BLUE_MASK))
                };
}

void HMDT::ltrim(std::string& str) {
    str.erase(str.begin(), str.begin() + str.find_first_not_of(" \t\n\r"));
}

void HMDT::rtrim(std::string& str) {
    str.erase(str.begin() + str.find_last_not_of(" \t\n\r") + 1, str.end());
}

void HMDT::trim(std::string& str) {
    ltrim(str);
    rtrim(str);
}

std::string HMDT::ltrim(const std::string& str) {
    std::string s = str;
    ltrim(s);
    return s;
}

std::string HMDT::rtrim(const std::string& str) {
    std::string s = str;
    rtrim(s);
    return s;
}

std::string HMDT::trim(const std::string& str) {
    std::string s = str;
    trim(s);
    return s;
}

std::string& HMDT::toLower(std::string& str) {
    return (str = toLower(const_cast<const std::string&>(str)));
}

std::string HMDT::toLower(const std::string& str) {
    auto lower_str = str;
    std::transform(str.begin(), str.end(), lower_str.begin(),
                   static_cast<int(*)(int)>(std::tolower));
    return lower_str;
}

std::string& HMDT::toUpper(std::string& str) {
    return (str = toUpper(const_cast<const std::string&>(str)));
}

std::string HMDT::toUpper(const std::string& str) {
    auto upper_str = str;
    std::transform(str.begin(), str.end(), upper_str.begin(),
                   static_cast<int(*)(int)>(std::toupper));
    return upper_str;
}

/**
 * @brief Converts the given XY coordinate to a flat index value
 *
 * @param image The image to index into
 * @param x The X coordinate to convert
 * @param y The Y coordinate to convert
 *
 * @return The given XY coordinate as a flat index
 */
uint64_t HMDT::xyToIndex(const BitMap* image, uint32_t x, uint32_t y) {
    return x + (y * image->info_header.width);
}

/**
 * @brief Converts the given XY coordinate to a flat index value
 *
 * @param w The width of the image
 * @param x The X coordinate to convert
 * @param y The Y coordinate to convert
 *
 * @return The given XY coordinate as a flat index
 */
uint64_t HMDT::xyToIndex(uint32_t w, uint32_t x, uint32_t y) {
    return x + (y * w);
}

/**
 * @brief Gets the color at the given XY coordinate from the given BitMap
 *
 * @paramm image The BitMap to get a color from
 * @param x The X coordinate to use
 * @param y The Y coordinate to use
 * @param depth The BitMap's color depth
 *
 * @return An RGB color from image at (x,y)
 */
HMDT::Color HMDT::getColorAt(const BitMap* image, uint32_t x, uint32_t y,
                             uint32_t depth)
{
    auto index = xyToIndex(image->info_header.width * depth, x * depth, y);

    return { image->data[index],
             image->data[index + 1],
             image->data[index + 2]
           };
}

/**
 * @brief Gets the color at the given XY coordinate from the given data
 *
 * @param dimensions The dimensions of the data
 * @param data The data
 * @param x The X coordinate to use
 * @param y The Y coordinate to use
 * @param depth The BitMap's color depth
 *
 * @return An RGB color from image at (x,y)
 */
HMDT::Color HMDT::getColorAt(const Dimensions& dimensions, const uint8_t* data,
                             uint32_t x, uint32_t y, uint32_t depth)
{
    auto index = xyToIndex(dimensions.w * depth, x * depth, y);

    return { data[index],
             data[index + 1],
             data[index + 2]
           };
}

auto HMDT::getColorAt(const BitMap2& image, uint32_t x, uint32_t y)
    -> Maybe<Color>
{
    auto depth = image.info_header.v1.bitsPerPixel / 8;
    auto index = xyToIndex(image.info_header.v1.width * depth, x * depth, y);

    auto res = getColorAt(image, index);
    RETURN_IF_ERROR(res);

    return res;
}

auto HMDT::getColorAt(const BitMap2& image, uint32_t index) -> Maybe<Color> {
    auto depth = image.info_header.v1.bitsPerPixel / 8;

    switch(depth) {
        case 3:
            return Color { image.data[index],
                           image.data[index + 1],
                           image.data[index + 2]
                         };
        case 2:
            RETURN_ERROR(STATUS_NOT_IMPLEMENTED);
        case 1:
            // 8-bit images require a color table
            if(image.color_table == nullptr) {
                WRITE_ERROR("8-bit bitmaps require a color table to be provided!");
                RETURN_ERROR(STATUS_COLOR_TABLE_REQUIRED);
            }

            // Rather than returning the data in the image, we instead will
            //   return the RGB data stored in the color table, at the index
            //   specified in the image data
            return Color { image.color_table[image.data[index]].red,
                           image.color_table[image.data[index]].green,
                           image.color_table[image.data[index]].blue,
                         };
        default:
            WRITE_ERROR("Unknown color depth ", depth);
            RETURN_ERROR(STATUS_NOT_IMPLEMENTED);
    }
}

/**
 * @brief Gets the given XY coordinate as a Pixel from the given BitMap
 *
 * @param image The BitMap to get a pixel from
 * @param x The X coordinate to use
 * @param y The Y coordinate to use
 *
 * @return All data related to the given XY coordinate as a Pixel
 */
HMDT::Pixel HMDT::getAsPixel(const BitMap* image, uint32_t x, uint32_t y,
                             uint32_t depth)
{
    return Pixel {
        { x, y },
        getColorAt(image, x, y, depth)
    };
}

auto HMDT::getAsPixel(const BitMap2& image, uint32_t x, uint32_t y)
    -> Maybe<Pixel>
{
    auto color = getColorAt(image, x, y);
    RETURN_IF_ERROR(color);

    return Pixel { { x, y }, color.value() };
}

/**
 * @brief Checks if two colors match
 *
 * @param c1 The first color
 * @param c2 The second color
 *
 * @return True if c1 matches c2
 */
bool HMDT::doColorsMatch(const Color& c1, const Color& c2) {
    return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}

/**
 * @brief Checks if a given point is within the bounds of the image.
 *
 * @param image The image
 * @param x The x coordinate to check
 * @param y The y coordinate to check
 *
 * @return True if the point is within the bounds of the image, false otherwise
 */
bool HMDT::isInImage(const Dimensions& dimensions, uint32_t x, uint32_t y) {
    return x < static_cast<uint32_t>(dimensions.w) &&
           y < static_cast<uint32_t>(dimensions.h);
}

bool HMDT::isShapeTooLarge(uint32_t s_width, uint32_t s_height,
                           const BitMap* image)
{
    auto i_width = image->info_header.width;
    auto i_height = image->info_header.height;

    // shape width/height >= (1/8) of the total map width/height
    return static_cast<float>(s_width) >= (i_width / 8.0f) ||
           static_cast<float>(s_height) >= (i_height / 8.0f);
}

/**
 * @brief Calculates the width and height of the given boundingg box
 *
 * @param bb The bounding box
 *
 * @return A pair containing the width and height of the bounding box
 */
std::pair<uint32_t, uint32_t> HMDT::calcDims(const BoundingBox& bb)
{
    return std::make_pair(static_cast<uint32_t>(std::abs(static_cast<int>(bb.top_right.x) - static_cast<int>(bb.bottom_left.x))),
                          static_cast<uint32_t>(std::abs(static_cast<int>(bb.top_right.y) - static_cast<int>(bb.bottom_left.y))));
}

/**
 * @brief Calculates the width and height of the given shape
 *
 * @param shape The shape
 *
 * @return A pair containing the width and height of the shape
 */
std::pair<uint32_t, uint32_t> HMDT::calcShapeDims(const Polygon& shape) {
    return calcDims(shape.bounding_box);
}

void HMDT::writeColorTo(unsigned char* color_data, uint32_t w,
                        uint32_t x, uint32_t y, Color c)
{
    uint32_t index = xyToIndex(w * 3, x * 3, y);

    // Make sure we swap B and R (because BMP format sucks)
    color_data[index] = c.b;
    color_data[index + 1] = c.g;
    color_data[index + 2] = c.r;
}

// Forward declaration
namespace HMDT {
    ProvinceType getProvinceType(const Color&);
}

auto HMDT::createProvincesFromShapeList(const PolygonList& shapes)
    -> ProvinceList
{
    ProvinceList provinces;
    provinces.reserve(shapes.size());
    for(uint32_t i = 0; i < shapes.size(); ++i) {
        auto&& shape = shapes[i];

        Color color = shape.color;

        auto prov_type = getProvinceType(color);

        UUID provinceID = shape.id;

        provinces[provinceID] = Province {
            provinceID,
            shape.unique_color,
            prov_type,
            false,
            "unknown",
            "None",
            0,
            shape.bounding_box,
            { },
            INVALID_PROVINCE  /* parent_id */,
            { } /* children */
        };
    }

    return provinces;
}

std::filesystem::path HMDT::getExecutablePath() {
    char path[PATH_MAX] = { 0 };
#ifdef _WIN32
    GetModuleFileName(NULL, path, PATH_MAX);
#else
    readlink("/proc/self/exe", path, PATH_MAX);
#endif

    return std::filesystem::path(path).parent_path();
}

/**
 * @brief Dump a demangled backtrace for the caller to 'out_file'
 *
 * @param out_file The FILE to write the backtrace to
 * @param max_frames The maximum number of frames to write
 */
void HMDT::dumpBacktrace(FILE* out_file, std::uint32_t max_frames, int tid) noexcept
{
    // Code taken/adapted from here:
    //   https://panthema.net/2008/0901-stacktrace-demangled/
    // and here:
    //   https://code.whatever.social/questions/5693192/win32-backtrace-from-c-code#5699483
    std::fprintf(out_file, "STACK TRACE (Max %u Frames) [TID:%d]:\n",
                 max_frames, tid);

    void* addr_list[max_frames + 1];

#ifdef _WIN32
    constexpr std::size_t MAX_SYMBOL_NAME_LENGTH = 1024;

    HANDLE process = GetCurrentProcess();

    SymInitialize(process, NULL, TRUE);

    // Skip the first frame since that is _this_ function.
    unsigned short frame_count = CaptureStackBackTrace(1 /* FramesToSkip */,
                                                       max_frames,
                                                       addr_list,
                                                       NULL /* BackTraceHash */);

    // Allocate enough space for a symbol with a name of MAX_SYMBOL_NAME_LENGTH 
    //   characters in length
    std::uint8_t symbol[sizeof(SYMBOL_INFO) + MAX_SYMBOL_NAME_LENGTH * sizeof(char)];
    PSYMBOL_INFO symbol_info = reinterpret_cast<PSYMBOL_INFO>(&symbol[0]);
    symbol_info->MaxNameLen = MAX_SYMBOL_NAME_LENGTH - 1; // Account for nul byte
    symbol_info->SizeOfStruct = sizeof(SYMBOL_INFO);

    // Output each name as:
    //   module : function+offset
    for(auto i = 0; i < frame_count; ++i) {
        DWORD64 displacement = 0;

        auto module_path = Log::getModulePath();

        auto result = SymFromAddr(process,
                                  (DWORD64)addr_list[i],
                                  &displacement,
                                  symbol_info);
        if (result == TRUE) {
            std::fprintf(out_file, "  %s : %s+0x%08x\n",
                         module_path.generic_string().c_str(),
                         symbol_info->Name,
                         displacement);
        } else {
            // demangling failed. Output just the address as-is
            std::fprintf(out_file, "  %s : %p\n",
                         module_path.generic_string().c_str(),
                         addr_list[i]);
        }
    }

#else
    int addr_len = backtrace(addr_list, sizeof(addr_list) / sizeof(void*));

    if(addr_len == 0) {
        std::fprintf(out_file, "  <No addresses returned! Possibly corrupt stack>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char** symbol_list = backtrace_symbols(addr_list, addr_len);
    RUN_AT_SCOPE_END(std::bind(std::free, symbol_list));

    std::size_t fname_size = 256;
    char* fname_buffer = (char*)malloc(fname_size);
    RUN_AT_SCOPE_END(std::bind(std::free, fname_buffer));

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for(auto i = 1; i < addr_len; ++i) {
        char* begin_name = 0;
        char* begin_offset = 0;
        char* end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for(char* p = symbol_list[i]; p != nullptr; ++p) {
            if(*p == '(') begin_name = p;
            else if(*p == '+') begin_offset = p;
            else if(*p == ')' && begin_offset != nullptr) {
                end_offset = p;
                break;
            }
        }

        if(begin_name != nullptr && begin_offset != nullptr &&
           end_offset != nullptr && begin_name < begin_offset)
        {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():
            int status = 0;
            char* ret = abi::__cxa_demangle(begin_name, fname_buffer,
                                            &fname_size, &status);
            if (status == 0) {
                fname_buffer = ret; // use possibly realloc()-ed string
                std::fprintf(out_file, "  %s : %s+%s\n", symbol_list[i],
                             fname_buffer, begin_offset);
            }
            else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                std::fprintf(out_file, "  %s : %s()+%s\n", symbol_list[i],
                             begin_name, begin_offset);
            }
        } else {
            // Failed to parse the line, so just dump it as-is
            std::fprintf(out_file, "  %s\n", symbol_list[i]);
        }
    }
#endif
}

/**
 * @brief Converts a pointer to a string using a common interface
 *
 * @param ptr The pointer
 *
 * @return A string representation of ptr as '0x{POINTER}'
 */
std::string std::to_string(const void* ptr) {
    std::stringstream ss;
    ss << ptr;
    return ss.str();
}

