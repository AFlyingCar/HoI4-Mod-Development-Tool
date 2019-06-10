
#include "UniqueColorGenerator.h"

#define NOMINMAX
#include <cmath>
#include <algorithm>

#include <iostream>

namespace {
    // Hue = [0..360]
    // Saturation = [0..1]
    // Lightness = [0..1]

    MapNormalizer::Color convertHSVToRGB(uint32_t hue, float saturation,
                                         float lightness)
    {
        auto chroma = (1 - (2 * lightness - 1)) * saturation;
        auto h_prime = hue / 60.0f;

        auto x = chroma * (1 - (std::fmod(h_prime, 2) - 1));

        auto m = lightness - (chroma / 2);

        MapNormalizer::Color c;
        if(0 <= h_prime && h_prime <= 1) {
            c = MapNormalizer::Color{static_cast<std::uint8_t>(255 * chroma), static_cast<std::uint8_t>(255 * x), 0};
        } else if(1 <= h_prime && h_prime <= 2) {
            c = MapNormalizer::Color{static_cast<std::uint8_t>(255 * x), static_cast<std::uint8_t>(255 * chroma), 0};
        } else if(2 <= h_prime && h_prime <= 3) {
            c = MapNormalizer::Color{0, static_cast<std::uint8_t>(255 * chroma), static_cast<std::uint8_t>(255 * x)};
        } else if(3 <= h_prime && h_prime <= 4) {
            c = MapNormalizer::Color{0, static_cast<std::uint8_t>(255 * x), static_cast<std::uint8_t>(255 * chroma)};
        } else if(4 <= h_prime && h_prime <= 5) {
            c = MapNormalizer::Color{static_cast<std::uint8_t>(255 * x), 0, static_cast<std::uint8_t>(255 * chroma)};
        } else if(5 <= h_prime && h_prime <= 6) {
            c = MapNormalizer::Color{static_cast<std::uint8_t>(255 * chroma), 0, static_cast<std::uint8_t>(255 * x)};
        } else {
            std::cerr << "[WARN] ~ Generated pixels of color value (0, 0, 0); "
                         "This should not happen." << std::endl;
            std::cerr << "\tchroma=" << chroma << '\n'
                      << "\th_prime=" << h_prime << '\n'
                      << "\tx=" << x << '\n'
                      << "\tm=" << m << '\n';

            c = MapNormalizer::Color{0, 0, 0};
        }

        c.r += m;
        c.g += m;
        c.b += m;

        return c;
    }

    const uint32_t HUE_MAX = 360; // 360 possible values
    const uint8_t SATURATION_MAX = 10; // Note: If this is too high, it may not
                                       //  increase the saturation enough every
                                       //  pass to be noticable
    const float SATURATION_STEP = (1.0f / SATURATION_MAX); // 0.01 -> 1 = 100 possible values
    const float LIGHTNESS = 0.5f; // Just leave this as a constant for now

    // total: 36000 possible colors
}

MapNormalizer::Color MapNormalizer::generateUniqueColor(uint32_t id) {
    std::cout << "id = " << id << std::endl;

    // hue: increments by 1 after every SATURATION_MAX generated values. repeats
    //      every HUE_MAX values
    // saturation: increments from 1 to SATURATION_MAX. Will be turned into
    //             a percentage. repeats every SATURATION_MAX valuesS

    // increment by 1 after the %, to prevent us from getting unwanted 0s
    auto c = convertHSVToRGB(((id / SATURATION_MAX) % HUE_MAX) + 1,
                             ((id % SATURATION_MAX) + 1) * SATURATION_STEP,
                             LIGHTNESS);

    std::cout << "Color = { " << (int)c.r << ',' << (int)c.g << ',' << (int)c.b
              << "}" << std::endl;

    return c;
}

MapNormalizer::Color MapNormalizer::generateUniqueColor(uint32_t id, Color bias)
{
    std::cout << "generateUniqueColor() with bias" << std::endl;
    std::cout << "id = " << id << std::endl;

    // hue: increments by 1 after every SATURATION_MAX generated values. repeats
    //      every HUE_MAX values
    // saturation: increments from 1 to SATURATION_MAX. Will be turned into
    //             a percentage. repeats every SATURATION_MAX valuesS

    // increment by 1 after the %, to prevent us from getting unwanted 0s
    auto c = convertHSVToRGB(((id / SATURATION_MAX) % HUE_MAX) + 1,
                             ((id % SATURATION_MAX) + 1) * SATURATION_STEP,
                             LIGHTNESS);

    std::cout << "Color = { " << (int)c.r << ',' << (int)c.g << ',' << (int)c.b
              << "}" << std::endl;

    return c;
}

