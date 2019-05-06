
#include "ShapeFinder.h"

#include <iostream>

#include "GraphicalDebugger.h" // writeDebugColor
#include "UniqueColorGenerator.h" // generateUniqueColor

// NOTE: This is a debug vector, in the future, we should have a function which
//   chooses new unique color values
static const std::vector<MapNormalizer::Color> UNIQUE_COLORS = {
    MapNormalizer::Color{ 0xFF, 0xFB, 0xFF },
    MapNormalizer::Color{ 0xF0, 0xFF, 0x0E },
    MapNormalizer::Color{ 0x0E, 0xF6, 0xFF },
    MapNormalizer::Color{ 0x3A, 0xFF, 0x2F },
    MapNormalizer::Color{ 0xF0, 0xFF, 0xFF },
    MapNormalizer::Color{ 0x5E, 0xFF, 0xFF },
    MapNormalizer::Color{ 0xFF, 0xFD, 0x00 },
    MapNormalizer::Color{ 0x5E, 0xFF, 0x0E },
    MapNormalizer::Color{ 0x0E, 0xFA, 0xF0 },
    MapNormalizer::Color{ 0x5E, 0xFF, 0xF0 },
    MapNormalizer::Color{ 0x0E, 0xFF, 0x0E },
    MapNormalizer::Color{ 0x5E, 0xF2, 0x0E },
    MapNormalizer::Color{ 0xF0, 0xFF, 0x5E },
};
static size_t next_color = 0;

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
 * @brief Checks if a given point is within the bounds of the image.
 *
 * @param image The image
 * @param x The x coordinate to check
 * @param y The y coordinate to check
 *
 * @return True if the point is within the bounds of the image, false otherwise
 */
bool MapNormalizer::isInImage(BitMap* image, uint32_t x, uint32_t y) {
    return x >= 0 && x < static_cast<uint32_t>(image->width) &&
           y >= 0 && y < static_cast<uint32_t>(image->height);
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
    return x + (y * image->width);
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
        { image->data[(x * 3) + (y * image->width * 3)],
          image->data[(x * 3) + (y * image->width * 3) + 1],
          image->data[(x * 3) + (y * image->width * 3) + 2]
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
 *
 * @return A list of all shapes in the BitMap image
 */
MapNormalizer::PolygonList MapNormalizer::findAllShapes(BitMap* image,
                                                        unsigned char* debug_data)
{
    PolygonList shapes;

    // Vector of whether or not we've visited a given index
    std::vector<bool> visited(image->width * image->height, false);
    Polygon next_shape;

    // A partitioned vector of pixels
    // +---------------------------------------+
    // |                     |                 |
    // | Non-boundary Pixels | Boundary Pixels |
    // |                     |                 |
    // +---------------------------------------+
    std::vector<Pixel> points;

    // Start off in the top left of the image
    points.insert(points.begin(), getAsPixel(image, 0, 0));

    // which index the boundary pixels start at
    size_t partition_idx = 1;

    std::cout << "Building shape #" << shapes.size() + 1 << std::endl;

    Color next_color;

findAllShapes_restart_loop:
    next_color = generateUniqueColor(shapes.size() + 1);

    while(!points.empty()) {
        // Pop the pixel off the top of the queue
        Pixel point = points.front();
        points.erase(points.begin());
        --partition_idx;

        writeDebugColor(debug_data, image->width, point.point.x, point.point.y,
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

                writeDebugColor(debug_data, image->width, p.point.x, p.point.y,
                                next_color);//UNIQUE_COLORS[next_color]);

                // Make sure it is marked as visited
                //   NOTE: Do we actually need to do this?
                visited[xyToIndex(image, p.point.x, p.point.y)] = true;
            }

            // ++next_color;

            partition_idx = 0;
            next_shape.u_color = next_color;//UNIQUE_COLORS[next_color];

            // Add this shape to the list of shapes and prepare it for receving
            //   the pixels in the next shape we look at
            shapes.push_back(next_shape);
            next_shape.pixels.clear();
            next_color = generateUniqueColor(shapes.size() + 1);

            std::cout << "Building shape #" << shapes.size() + 1 << std::endl;
        }

        // Is the shape to the left of the current pixel still viable to look at?
        uint32_t index = xyToIndex(image, point.point.x + 1, point.point.y);
        if(isInImage(image, point.point.x + 1, point.point.y) &&
           !visited[index])
        {
            writeDebugColor(debug_data, image->width, point.point.x + 1, point.point.y,
                            Color{0, 0, 0xFF});

            // Mark that this pixel is no longer viable
            visited[index] = true;

            auto pix = getAsPixel(image, point.point.x + 1, point.point.y);

            // Add it to before or after the partition depending on if its a
            //   boundary pixel or not
            if(isBoundaryPixel(pix)) {
                points.push_back(pix);
            } else {
                points.insert(points.begin() + (partition_idx++), pix);
            }
        }

        // Is the shape to the right of the current pixel still viable to look at?
        index = xyToIndex(image, point.point.x - 1, point.point.y);
        if(isInImage(image, point.point.x - 1, point.point.y) &&
           !visited[index])
        {
            writeDebugColor(debug_data, image->width, point.point.x - 1, point.point.y,
                            Color{0, 0, 0xFF});

            visited[index] = true;

            auto pix = getAsPixel(image, point.point.x - 1, point.point.y);

            if(isBoundaryPixel(pix)) {
                points.push_back(pix);
            } else {
                points.insert(points.begin() + (partition_idx++), pix);
            }
        }

        index = xyToIndex(image, point.point.x, point.point.y + 1);
        if(isInImage(image, point.point.x, point.point.y + 1)
           && !visited[index])
        {
            writeDebugColor(debug_data, image->width, point.point.x, point.point.y + 1,
                            Color{0, 0, 0xFF});

            visited[index] = true;
            auto pix = getAsPixel(image, point.point.x, point.point.y + 1);

            if(isBoundaryPixel(pix)) {
                points.push_back(pix);
            } else {
                points.insert(points.begin() + (partition_idx++), pix);
            }
        }

        index = xyToIndex(image, point.point.x, point.point.y - 1);
        if(isInImage(image, point.point.x, point.point.y - 1) &&
           !visited[index])
        {
            writeDebugColor(debug_data, image->width, point.point.x, point.point.y - 1,
                            Color{0, 0, 0xFF});

            visited[index] = true;
            auto pix = getAsPixel(image, point.point.x, point.point.y - 1);

            if(isBoundaryPixel(pix)) {
                points.push_back(pix);
            } else {
                points.insert(points.begin() + (partition_idx++), pix);
            }
        }

        next_shape.pixels.push_back(point);
        writeDebugColor(debug_data, image->width, point.point.x, point.point.y,
                        next_color);//UNIQUE_COLORS[next_color]);
    }

    // Check for pixels that were missed in the last pass

    size_t x = 0;
    size_t y = 0;

    std::cout << "Checking for pixels we may have missed." << std::endl;
    for(auto index = 0; index < visited.size(); ++index) {
        // Index -> XY conversion
        x = (x + 1) % image->width;
        y += (x == 0 ? 1 : 0);

        if(!visited[index]) {
            auto p = getAsPixel(image, x, y);

            visited[index] = true;

            if(isBoundaryPixel(p)) {
                std::cout << "Non-visited boundary point found." << std::endl;

                // Find the first shape which has a pixel adjacent to this one
                for(auto&& s : shapes) {
                    for(auto&& pix : s.pixels) {
                        if(isAdjacent(pix, x, y)) {
                            // Add this pixel that shape
                            s.pixels.push_back(p);
                            goto end_double_for_loop;
                        }
                    }
                }
end_double_for_loop:;
            } else {
                std::cout << "Found a pixel we missed at (" << x << ',' << y
                          << ")." << std::endl;
                // Reset the state back to the beginning and jump to the top
                //++next_color;
                partition_idx = 1;
                points.insert(points.begin(), p);
                next_shape.pixels.clear();
                next_shape.u_color = next_color = generateUniqueColor(shapes.size() + 1);//UNIQUE_COLORS[next_color];

                writeDebugColor(debug_data, image->width, p.point.x, p.point.y,
                                next_color);//UNIQUE_COLORS[next_color]);

                goto findAllShapes_restart_loop;
            }
        }
    }

    if(!next_shape.pixels.empty())
        shapes.push_back(next_shape);

    return shapes;
}

