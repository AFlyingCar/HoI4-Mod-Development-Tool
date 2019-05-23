
#include "Util.h"

// Helper replacement for __builtin_ctz if on MSVC
#ifdef MSC_VER
# include <intrin>

// https://stackoverflow.com/a/20468180
static std::uint32_t __inline __builtin_ctz(std::uint32_t value) {
    DWORD trailing_zero = 0;

    if(_BitScanForward(&trailing_zero, value)) return trailing_zero;
    return 0;
}
#endif

// https://stackoverflow.com/a/31393298
std::uint32_t MapNormalizer::indexOfLSB(std::uint32_t value) {
    return __builtin_ctz(value);// + 1;
}

