/**
 * @file WorldNormalBuilder.cpp
 *
 * @brief Defines functions for building the world normal map.
 *
 * @par A lot of this code was adapted from answers to this StackOverflow
 *      question:
 *      https://stackoverflow.com/questions/2368728/can-normal-maps-be-generated-from-a-texture
 */

#include "WorldNormalBuilder.h"

#include <cmath> // std::sqrt

#include "BitMap.h"
#include "Util.h"
#include "Types.h"

namespace {
    /**
     * @brief Defines a simple Vec3 type.
     * @details This is here rather than in a header file since nowhere else in
     *          this program is a Vec3 needed.
     */
    struct Vec3 {
        double x; //!< The X coordinate
        double y; //!< The Y coordinate
        double z; //!< The Z coordinate
    };

    /**
     * @brief Calculate the intensity of the given color.
     *
     * @param c The color
     *
     * @return The intensity
     */
    double intensity(HMDT::Color c) {
        return ((c.r + c.g + c.b) / 3.0) / 255.0;
    }

    /**
     * @brief Normalizes a 3D-vector
     *
     * @param x The X coordinate of the vector
     * @param y The Y coordinate of the vector
     * @param z The Z coordinate of the vector
     * 
     * @return The unit vector of <x, y, z>
     */
    Vec3 normalize(double x, double y, double z) {
        auto l = std::sqrt(x*x + y*y + z*z);

        return Vec3{ x / l, y / l, z / l };
    }
}

/**
 * @brief Generates a world normal map from the given input.
 *
 * @param heightmap The input heightmap to generate a normal map from.
 * @param normal_data The output image data array. All normal map information
 *                    will be written here.
 */
void HMDT::generateWorldNormalMap(BitMap* heightmap,
                                           unsigned char* normal_data)
{
    auto width = heightmap->info_header.width;
    auto height = heightmap->info_header.height;

    // For every single pixel
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            // Index into normal_data based on our current X,Y coordinate
            // Hardcode 3 because we want the output to have a pixel-depth of 3
            auto index = xyToIndex(width * 3, x * 3, y);

            // Surrounding pixels
            Pixel top_left  = getAsPixel(heightmap, clamp(x - 1, 0, width), clamp(y - 1, 0, height), 1);
            Pixel top       = getAsPixel(heightmap, x, clamp(y - 1, 0, height), 1);
            Pixel top_right = getAsPixel(heightmap, clamp(x + 1, 0, width), clamp(y - 1, 0, height), 1);
            Pixel left      = getAsPixel(heightmap, clamp(x - 1, 0, width), y, 1);
            Pixel right     = getAsPixel(heightmap, clamp(x + 1, 0, width), y, 1);
            Pixel bot_left  = getAsPixel(heightmap, clamp(x - 1, 0, width), clamp(y + 1, 0, height), 1);
            Pixel bot       = getAsPixel(heightmap, x, clamp(y + 1, 0, height), 1);
            Pixel bot_right = getAsPixel(heightmap, clamp(x + 1, 0, width), clamp(y + 1, 0, height), 1);

            // Get intensities of surrounding pixels
            double tl_intensity = intensity(top_left.color);
            double t_intensity = intensity(top.color);
            double tr_intensity = intensity(top_right.color);
            double l_intensity = intensity(left.color);
            double r_intensity = intensity(right.color);
            double bl_intensity = intensity(bot_left.color);
            double b_intensity = intensity(bot.color);
            double br_intensity = intensity(bot_right.color);

            // sobel filter
            double dX = (tr_intensity + 2.0 * r_intensity + br_intensity) - (tl_intensity + 2.0 * l_intensity + bl_intensity);
            double dY = (bl_intensity + 2.0 * b_intensity + br_intensity) - (tl_intensity + 2.0 * t_intensity + tr_intensity);
            double dZ = 1.0 / 2.0;

            // Normalize the sobel filter values
            auto [nX, nY, nZ] = normalize(dX, dY, dZ);

            // Convert the coordinates back into color values
            auto r = static_cast<std::uint8_t>((nX + 1.0) * (255.0 / 2.0));
            auto g = static_cast<std::uint8_t>((nY + 1.0) * (255.0 / 2.0));
            auto b = static_cast<std::uint8_t>((nZ + 1.0) * (255.0 / 2.0));

            // Finally, place the new color data into the output array.
            //  Note: Make sure that B and R are swapped (because .BMP format)
            normal_data[index] = b;
            normal_data[index + 1] = g;
            normal_data[index + 2] = r;
        }
    }
}

