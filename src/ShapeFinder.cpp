
#include "ShapeFinder.h"

#include <iostream>
#include <sstream>
#include <cmath> // std::sqrt
#include <list>
#include <map>

#include "GraphicalDebugger.h" // writeDebugColor
#include "UniqueColorGenerator.h" // generateUniqueColor
#include "ProvinceMapBuilder.h"
#include "Constants.h"
#include "Util.h"
#include "Logger.h"

std::vector<MapNormalizer::Pixel> MapNormalizer::problematic_pixels;

/**
 * @brief Checks if the given pixel is a boundary pixel.
 *
 * @param p The pixel to check
 * @return True if p is a boundary pixel, false otherwise.
 */
bool MapNormalizer::isBoundaryPixel(Pixel p) {
    return p.color.r == 0 && p.color.g == 0 && p.color.b == 0;
}


/**
 * @brief Checks if two colors match
 *
 * @param c1 The first color
 * @param c2 The second color
 *
 * @return True if c1 matches c2
 */
bool MapNormalizer::doColorsMatch(Color c1, Color c2) {
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
bool MapNormalizer::isInImage(BitMap* image, uint32_t x, uint32_t y) {
    return x >= 0 && x < static_cast<uint32_t>(image->info_header.width) &&
           y >= 0 && y < static_cast<uint32_t>(image->info_header.height);
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
uint32_t MapNormalizer::xyToIndex(BitMap* image, uint32_t x, uint32_t y) {
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
uint32_t MapNormalizer::xyToIndex(uint32_t w, uint32_t x, uint32_t y) {
    return x + (y * w);
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
MapNormalizer::Pixel MapNormalizer::getAsPixel(BitMap* image, uint32_t x,
                                               uint32_t y)
{
    return Pixel {
        { x, y },
        { image->data[(x * 3) + (y * image->info_header.width * 3)],
          image->data[(x * 3) + (y * image->info_header.width * 3) + 1],
          image->data[(x * 3) + (y * image->info_header.width * 3) + 2]
        }
    };
}

/**
 * @brief Checks if a given Pixel is adjacent to the given XY coordinate
 *
 * @param p The pixel to check
 * @param x The X coordinate to check
 * @param y The Y coordinate to check
 *
 * @return True if p is adjacent to XY
 */
bool MapNormalizer::isAdjacent(const Pixel& p, size_t x, size_t y) {
    return (p.point.x == (x + 1) && p.point.y == y) ||
           (p.point.x == (x - 1) && p.point.y == y) ||
           (p.point.x == x && p.point.y == (y + 1)) ||
           (p.point.x == x && p.point.y == (y - 1));
}

/**
 * @brief Finds all shapes in a given BitMap image
 *
 * @param image The image to get all shapes from
 * @param debug_data A flat array of color values to write to for debugging. May
 *                   be NULL
 * @param problem_pixels A vector to be filled with pixels that have caused either
 *                       an error or a warning.
 *
 * @return A list of all shapes in the BitMap image
 */
MapNormalizer::PolygonList MapNormalizer::findAllShapes(BitMap* image,
                                                        unsigned char* debug_data,
                                                        std::vector<Pixel>& problem_pixels)
{
    PolygonList shapes;

    using namespace std::string_literals;

    // Vector of whether or not we've visited a given index
    std::vector<bool> visited(image->info_header.width * image->info_header.height, false);
    Polygon next_shape;

    // A partitioned vector of pixels
    // +---------------------------------------+
    // |                     |                 |
    // | Non-boundary Pixels | Boundary Pixels |
    // |                     |                 |
    // +---------------------------------------+
    std::list<Pixel> points;

    // Start off in the top left of the image
    auto partition_idx = points.insert(points.begin(), getAsPixel(image, 0, 0));

    // Original Color -> Pixels with this color
    std::map<std::uint32_t, std::vector<Pixel>> read_color_amounts;

    auto finalize_shape = [&read_color_amounts, &shapes, &problem_pixels, &debug_data, &image](Polygon& shape)
    {
        // Determine what the original color was
        auto orig_color = read_color_amounts.begin()->first;
        if(read_color_amounts.size() > 1) {
            writeWarning("Found more than 1 color in the shape. Choosing the most common color.");

            auto most = read_color_amounts.begin()->second.size();
            for(auto&& [color,dup_pixels] : read_color_amounts) {
                // Ignore border colors
                if(color == 0) continue;

                problem_pixels.insert(problem_pixels.end(),
                                      dup_pixels.begin(), dup_pixels.end());

                std::stringstream ss;
                ss << std::hex << color;
                writeWarning("\t0x"s + ss.str() +
                             " appears " + std::to_string(dup_pixels.size()) +
                             " times.");

                if(most < dup_pixels.size())
                    orig_color = color;
            }
        }

        // Determine what the province type of this shape was based on the color
        //   it was read in as
        auto prov_type = getProvinceType(orig_color).first;

        shape.color = RGBToColor(orig_color);

        // Generate a unique color for this shape based on the province type
        shape.unique_color = generateUniqueColor(prov_type);

        std::stringstream ss;
        ss << "0x" << std::hex << orig_color << " -> " << prov_type << " -> 0x" << shape.unique_color;
        writeDebug(ss.str());

        // Redraw the shape with the new unique color
        // // Redraw the shape with the new unique color
        for(auto&& pixel : shape.pixels)
            writeDebugColor(debug_data, image->info_header.width, pixel.point.x,
                            pixel.point.y, shape.unique_color);

        // Add this shape to the list of shapes and prepare it for receving
        //   the pixels in the next shape we look at
        shapes.push_back(shape);

        shape.pixels.clear();
        read_color_amounts.clear();
    };

        auto point_check = [&](uint32_t x, uint32_t y) {
            uint32_t index = xyToIndex(image, x, y);

            // Is the pixel still viable to look at?
            if(isInImage(image, x, y) && !visited[index]) {
                writeDebugColor(debug_data, image->info_header.width, x, y,
                                Color{0, 0, 0xFF});

                // Mark that this pixel is no longer viable
                visited[index] = true;

                auto pix = getAsPixel(image, x, y);

                if(x == 209 && y == 666) {
                    std::cout << "Point (209,666) has color (" << pix.color.r << ',' << pix.color.g << ',' << pix.color.b << ")" << std::endl;
                }

                // Add it to before or after the partition depending on if its a
                //   boundary pixel or not
                if(isBoundaryPixel(pix)) {
                    points.push_back(pix);
                } else {
                    partition_idx = points.insert(partition_idx, pix);
                }
            }
        };

findAllShapes_restart_loop:
    while(!points.empty()) {
        // Pop the pixel off the top of the queue
        Pixel point = points.front();
        partition_idx = points.erase(points.begin());

        writeDebugColor(debug_data, image->info_header.width, point.point.x, point.point.y,
                        Color{0xFF, 0, 0});

        // If we find a boundary pixel, then that means that there are no
        //   non-boundary pixels left in the queue. Therefore, there are no more
        //   pixels left which do not make up this shape
        if(isBoundaryPixel(point)) {
            // Grab every boundary pixel left in the queue, and add it to the
            //   current shape
            while(!points.empty()) {
                Pixel p = points.front();
                points.erase(points.begin());
                next_shape.pixels.push_back(p);

                writeDebugColor(debug_data, image->info_header.width, p.point.x, p.point.y,
                                DEBUG_COLOR);

                // Make sure it is marked as visited
                //   NOTE: Do we actually need to do this?
                visited[xyToIndex(image, p.point.x, p.point.y)] = true;
            }

            partition_idx = points.begin();

            writeDebug("Shape detected with "s +
                       std::to_string(read_color_amounts.size()) +
                       " different color(s) and " +
                       std::to_string(next_shape.pixels.size()) + " pixels.");

            if(next_shape.pixels.size() < 10) {
                writeWarning("Tiny province detected! Either your input image "
                             "is wrong or there is a bug! Please check the "
                             "following pixels and press any key to continue "
                             "execution, or CTRL+C to quit.");
                deleteInfoLine();

                std::cerr << "Pixels = {" << std::endl;
                for(auto pix : next_shape.pixels) {
                    problem_pixels.push_back(pix);
                    shapes.back().pixels.push_back(pix);
                    writeDebugColor(debug_data, image->info_header.width,
                                    pix.point.x, pix.point.y,
                                    shapes.back().unique_color);
                    std::cerr << "\t(" << pix.point.x << ',' << pix.point.y
                              << ')' << std::endl;
                }
                std::cerr << '}' << std::endl;

                // std::getchar();
            } else {
                setInfoLine("Building shape #" + std::to_string(shapes.size() + 1));
                finalize_shape(next_shape);
            }
        } else {
            // Make sure we do the counting here, so we don't include
            //   boundary pixels
            read_color_amounts[colorToRGB(point.color)].push_back(point);

            if(colorToRGB(point.color) == 0) {
                std::stringstream ss;
                ss << "Pixel (" << point.point.x << ',' << point.point.y
                   << ") has color value of 0x0. This should not happen, and "
                      "can cause weird side-effects with CSV and image "
                      "generation.";
                writeWarning(ss.str());
            }

            //    << ") has color 0x" << std::hex << colorToRGB(point.color)
            //    << ". Pixel #" << std::dec
            //    << read_color_amounts[colorToRGB(point.color)]
            //    << " with this color.";
            // writeDebug(ss.str());
        }

        point_check(point.point.x + 1, point.point.y); // left
        point_check(point.point.x - 1, point.point.y); // right
        point_check(point.point.x, point.point.y + 1); // up
        point_check(point.point.x, point.point.y - 1); // down

        next_shape.pixels.push_back(point);
        writeDebugColor(debug_data, image->info_header.width, point.point.x, point.point.y,
                        DEBUG_COLOR);
    }

    // Check for pixels that were missed in the last pass
    writeStdout("Checking for pixels we may have missed...");
    for(auto index = 0; index < visited.size(); ++index) {
        // Index -> XY conversion
        // Note: We need to subtract 1 from x since this calculation puts as
        //   always at 1 after index
        size_t x = index % image->info_header.width;
        size_t y = index / image->info_header.width;

        if(!visited[index]) {
            auto p = getAsPixel(image, x, y);

            visited[index] = true;

            if(isBoundaryPixel(p)) {
                writeStdout("Non-visited boundary point found at (" +
                            std::to_string(x) + ',' + std::to_string(y) + ").");

                // Find the first shape which has a pixel adjacent to this one
                for(auto&& s : shapes) {
                    for(auto&& pix : s.pixels) {
                        if(isAdjacent(pix, x, y)) {
                            // Add this pixel that shape
                            writeDebugColor(debug_data, image->info_header.width, x, y, s.unique_color);
                            s.pixels.push_back(p);
                            goto end_double_for_loop;
                        }
                    }
                }
end_double_for_loop:;
            } else {
                writeStdout("Found a pixel we missed at (" + std::to_string(x) +
                            ',' + std::to_string(y) + ").");

                // First finish completing the last shape we were working on
                // Note: we don't need to reset points here, since us just being
                //   here means that it should already be empty
                finalize_shape(next_shape);

                // Reset the state back to the beginning and jump to the top
                partition_idx = points.insert(points.begin(), p);

                writeDebugColor(debug_data, image->info_header.width, p.point.x, p.point.y,
                                DEBUG_COLOR);

                goto findAllShapes_restart_loop;
            }
        }
    }

    if(!next_shape.pixels.empty()) {
        finalize_shape(next_shape);
    }

    return shapes;
}

