
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
#include "Options.h"

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
 * @param province_data A flat array of color values to write to for the province map.
 * @param river_data A flat array of color values to write to for the blank river map.
 * @param problem_pixels A vector to be filled with pixels that have caused either
 *                       an error or a warning.
 *
 * @return A list of all shapes in the BitMap image
 */
MapNormalizer::PolygonList MapNormalizer::findAllShapes(BitMap* image,
                                                        unsigned char* province_data,
                                                        unsigned char* river_data,
                                                        std::vector<Pixel>& problem_pixels)
{
    PolygonList shapes;

    using namespace std::string_literals;

    // Vector of whether or not we've visited a given index
    std::vector<bool> visited(image->info_header.width * image->info_header.height, false);
    Polygon next_shape;

    // Start at the opposite corners of the image
    next_shape.bottom_left = Point2D{
        static_cast<uint32_t>(image->info_header.width),
        static_cast<uint32_t>(image->info_header.height)
    };
    next_shape.top_right = Point2D{ 0, 0 };

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

    auto colorize_pixel = [&province_data, &river_data, &image](const Pixel& pixel,
                                                                const Color& ucolor,
                                                                const Color& rcolor)
    {
        // Redraw the shape with the new unique color into province_data
        writeDebugColor(province_data, image->info_header.width, pixel.point.x,
                        pixel.point.y, ucolor);

        // Draw default map data for the shape into river_data
        writeDebugColor(river_data, image->info_header.width, pixel.point.x,
                        pixel.point.y, rcolor);
    };

    auto finalize_shape = [&read_color_amounts, &shapes, &problem_pixels,
                           &colorize_pixel, &image](Polygon& shape)
    {
        // Determine what the original color was
        auto orig_color = read_color_amounts.begin()->first;
        if(read_color_amounts.size() > 1) {
            writeWarning("Found more than 1 color in the shape. Choosing the most common color.");

            auto most = read_color_amounts.begin()->second.size();
            for(auto&& [color,dup_pixels] : read_color_amounts) {
                checkForPause();

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

        // Check for minimum province size.
        //  See: https://hoi4.paradoxwikis.com/Map_modding
        if(shape.pixels.size() <= MIN_SHAPE_SIZE) {
            // If there is only one pixel, then most likely it is a result of 
            //  the bug which sometimes skips a pixel.
            // TODO: This is a terrible solution, as it doesn't actually solve
            //  the problem, but it should work in all cases that this might
            //  come up.
            if(shape.pixels.size() == 1) {
                auto pixel = shape.pixels.at(0);
                std::stringstream ss;
                ss << "Encountered known bug causing some pixels to not join "
                      "the right shape. Going to try and merge them anyway. "
                      "Verify pixel " << pixel.point << '.';
                writeWarning(ss.str());

                shapes.back().pixels.push_back(pixel);

                auto prov_type = getProvinceType(shapes.back().color);
                Color river_map_color = (prov_type == ProvinceType::LAND) ? 
                                            Color{ 0xFF, 0xFF, 0xFF } :
                                            Color{ 0x7A, 0x7A, 0x7A };

                // Finish up what the rest of the function would have done to
                //  this one pixel
                colorize_pixel(pixel, shapes.back().unique_color, river_map_color);

                shape.pixels.clear();
                read_color_amounts.clear();
                return;
            } else {
                std::stringstream ss;
                ss << "Shape #" << shapes.size() << " has only "
                   << shape.pixels.size() << " pixels. The minimum is "
                   << MIN_SHAPE_SIZE << ". Check your input file!";
                writeWarning(ss.str());
            }
        }

        if(auto [width, height] = calcShapeDims(shape); isShapeTooLarge(width,
                                                                        height,
                                                                        image))
        {
            std::stringstream ss;
            ss << "Shape #" << shapes.size() << " has a bounding box of size ("
               << width << ',' << height << "). One of these is larger than the"
               << " allowed ratio of 1/8 * ("
               << image->info_header.width << ',' << image->info_header.height
               << ") => (" << (image->info_header.width / 8.0f) << ','
               << (image->info_header.height / 8.0f) << "). Check the province "
                                                        "borders.";
            ss << " Bounds are: " << shape.bottom_left << " to " << shape.top_right;
            writeWarning(ss.str());
        }

        // TODO: Should we check here for invalid X crossing?

        // Determine what the province type of this shape was based on the color
        //   it was read in as
        auto prov_type = getProvinceType(orig_color);

        shape.color = RGBToColor(orig_color);

        // Generate a unique color for this shape based on the province type
        shape.unique_color = generateUniqueColor(prov_type);

        if(prog_opts.verbose) {
            std::stringstream ss;
            ss << "0x" << std::hex << orig_color << " -> " << prov_type << " -> 0x" << shape.unique_color;
            writeDebug(ss.str());
        }

        Color river_map_color = (prov_type == ProvinceType::LAND) ? 
                                    Color{ 0xFF, 0xFF, 0xFF } :
                                    Color{ 0x7A, 0x7A, 0x7A };

        for(auto&& pixel : shape.pixels) {
            checkForPause();
            colorize_pixel(pixel, shape.unique_color, river_map_color);

        }

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
            writeDebugColor(province_data, image->info_header.width, x, y,
                            Color{0, 0, 0xFF});

            // Mark that this pixel is no longer viable
            visited[index] = true;

            auto pix = getAsPixel(image, x, y);

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
        checkForPause();

        // Pop the pixel off the top of the queue
        Pixel point = points.front();
        partition_idx = points.erase(points.begin());

        visited[xyToIndex(image, point.point.x, point.point.y)] = true;

        writeDebugColor(province_data, image->info_header.width, point.point.x, point.point.y,
                        Color{0xFF, 0, 0});

        // If we find a boundary pixel, then that means that there are no
        //   non-boundary pixels left in the queue. Therefore, there are no more
        //   pixels left which do not make up this shape
        if(isBoundaryPixel(point)) {
            // Grab every boundary pixel left in the queue, and add it to the
            //   current shape
            while(!points.empty()) {
                checkForPause();

                Pixel p = points.front();
                points.erase(points.begin());
                next_shape.pixels.push_back(p);

                // Set up the bounding box of this shape
                if(p.point.x > next_shape.top_right.x) {
                    next_shape.top_right.x = p.point.x;
                } else if(p.point.x < next_shape.bottom_left.x) {
                    next_shape.bottom_left.x = p.point.x;
                }

                if(p.point.y > next_shape.top_right.y) {
                    next_shape.top_right.y = p.point.y;
                } else if(p.point.y < next_shape.bottom_left.y) {
                    next_shape.bottom_left.y = p.point.y;
                }

                writeDebugColor(province_data, image->info_header.width, p.point.x, p.point.y,
                                DEBUG_COLOR);

                // Make sure it is marked as visited
                //   NOTE: Do we actually need to do this?
                visited[xyToIndex(image, p.point.x, p.point.y)] = true;
            }

            // Make absolutely-doubly sure that it is in fact empty
            points.clear();

            partition_idx = points.begin();

            if(prog_opts.verbose)
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

                auto prov_type = getProvinceType(shapes.back().color);
                Color river_map_color = (prov_type == ProvinceType::LAND) ? 
                                            Color{ 0xFF, 0xFF, 0xFF } :
                                            Color{ 0x7A, 0x7A, 0x7A };

                std::cerr << "Pixels = {" << std::endl;
                for(auto pix : next_shape.pixels) {
                    problem_pixels.push_back(pix);
                    shapes.back().pixels.push_back(pix);

                    colorize_pixel(pix, shapes.back().unique_color, river_map_color);

                    std::cerr << "\t(" << pix.point.x << ',' << pix.point.y
                              << ')' << std::endl;
                }
                std::cerr << '}' << std::endl;

                shapes.back().pixels.push_back(point);
                visited[xyToIndex(image, point.point.x, point.point.y)] = true;

                // std::getchar();
            } else {
                if(!prog_opts.quiet)
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

        // Set up the bounding box of this shape
        if(point.point.x < next_shape.top_right.x) {
            next_shape.top_right.x = point.point.x;
        } else if(point.point.x > next_shape.bottom_left.x) {
            next_shape.bottom_left.x = point.point.x;
        }

        if(point.point.y < next_shape.top_right.y) {
            next_shape.top_right.y = point.point.y;
        } else if(point.point.y > next_shape.bottom_left.y) {
            next_shape.bottom_left.y = point.point.y;
        }

        writeDebugColor(province_data, image->info_header.width, point.point.x, point.point.y,
                        DEBUG_COLOR);
    }

    checkForPause();

    // Check for pixels that were missed in the last pass
    if(!prog_opts.quiet)
        writeStdout("Checking for pixels we may have missed...");
    for(auto index = 0; index < visited.size(); ++index) {
        checkForPause();

        // Index -> XY conversion
        // Note: We need to subtract 1 from x since this calculation puts as
        //   always at 1 after index
        size_t x = index % image->info_header.width;
        size_t y = index / image->info_header.width;

        if(!visited[index]) {
            auto p = getAsPixel(image, x, y);

            visited[index] = true;

            if(isBoundaryPixel(p)) {
                if(prog_opts.verbose)
                    writeStdout("Non-visited boundary point found at (" +
                                std::to_string(x) + ',' + std::to_string(y) + ").");

                // Find the first shape which has a pixel adjacent to this one
                for(auto&& s : shapes) {
                    auto prov_type = getProvinceType(s.color);
                    Color river_map_color = (prov_type == ProvinceType::LAND) ? 
                                                Color{ 0xFF, 0xFF, 0xFF } :
                                                Color{ 0x7A, 0x7A, 0x7A };

                    for(auto&& pix : s.pixels) {
                        if(isAdjacent(pix, x, y)) {
                            // Add this pixel that shape
                            colorize_pixel(p, s.unique_color, river_map_color);

                            s.pixels.push_back(p);
                            goto end_double_for_loop;
                        }
                    }
                }
end_double_for_loop:;
            } else {
                if(prog_opts.verbose)
                    writeStdout("Found a pixel we missed at (" + std::to_string(x) +
                                ',' + std::to_string(y) + ").");

                // First finish completing the last shape we were working on
                // Note: we don't need to reset points here, since us just being
                //   here means that it should already be empty
                finalize_shape(next_shape);

                // Reset the state back to the beginning and jump to the top
                partition_idx = points.insert(points.begin(), p);

                writeDebugColor(province_data, image->info_header.width, p.point.x, p.point.y,
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

