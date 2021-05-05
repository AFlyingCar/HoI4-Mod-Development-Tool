
#include "ShapeFinder2.h"

#include <sstream>

#include "Logger.h"
#include "Util.h"
#include "Constants.h"
#include "ProvinceMapBuilder.h" // getProvinceType
#include "UniqueColorGenerator.h" // generateUniqueColor
#include "Options.h"
#include "GraphicalDebugger.h"

/**
 * @brief Constructs a ShapeFinder
 *
 * @param image The image to detect shapes from
 */
MapNormalizer::ShapeFinder::ShapeFinder(const BitMap* image):
    m_image(image),
    m_label_matrix_size(m_image->info_header.width *
                        m_image->info_header.height),
    m_label_matrix(new uint32_t[m_label_matrix_size]),
    m_label_parents(),
    m_border_pixels(),
    m_label_to_color(),
    m_do_estop(false),
    m_stage(Stage::START),
    m_shapes()
{
}

MapNormalizer::ShapeFinder::ShapeFinder():
    m_image(nullptr),
    m_label_matrix_size(0),
    m_label_matrix(nullptr),
    m_label_parents(),
    m_border_pixels(),
    m_label_to_color(),
    m_do_estop(false),
    m_stage(Stage::START),
    m_shapes()
{ }

MapNormalizer::ShapeFinder::ShapeFinder(ShapeFinder&& other):
    m_image(std::move(other.m_image)),
    m_label_matrix_size(std::move(other.m_label_matrix_size)),
    m_label_matrix(std::move(other.m_label_matrix)),
    m_label_parents(std::move(other.m_label_parents)),
    m_border_pixels(std::move(other.m_border_pixels)),
    m_label_to_color(std::move(other.m_label_to_color)),
    m_do_estop(std::move(other.m_do_estop)),
    m_stage(std::move(other.m_stage)),
    m_shapes(std::move(other.m_shapes))
{ }

auto MapNormalizer::ShapeFinder::operator=(ShapeFinder&& other) -> ShapeFinder&
{
    m_image = std::move(other.m_image);
    m_label_matrix_size = std::move(other.m_label_matrix_size);
    m_label_matrix = std::move(other.m_label_matrix);
    m_label_parents = std::move(other.m_label_parents);
    m_border_pixels = std::move(other.m_border_pixels);
    m_label_to_color = std::move(other.m_label_to_color);
    m_do_estop = std::move(other.m_do_estop);
    m_stage = std::move(other.m_stage);
    m_shapes = std::move(other.m_shapes);

    return *this;
}

/**
 * @brief Performs the first pass of the Connected-Component-Labeling (CCL) algorithm
 *
 * @return The total number of border pixels found.
 */
uint32_t MapNormalizer::ShapeFinder::pass1() {
    uint32_t width = m_image->info_header.width;
    uint32_t height = m_image->info_header.height;

    uint32_t next_label = 1;

    uint32_t num_border_pixels = 0;

    if(!prog_opts.quiet)
        writeStdout("Performing Pass #1 of CCL.");

    auto& worker = GraphicsWorker::getInstance();

    // Go over every pixel of the image
    for(uint32_t y = 0; y < height; ++y) {
        for(uint32_t x = 0; x < width; ++x) {
            if(m_do_estop) {
                return 0;
            }

            Color color = getColorAt(m_image, x, y);
            uint32_t index = xyToIndex(m_image, x, y);
            uint32_t& label = m_label_matrix[index] = next_label;

            if(!prog_opts.quiet)
                setInfoLine("Pixel ", Point2D{x, y}," [", color, "]");

            // Skip this pixel if it is part of a border
            if(color == BORDER_COLOR) {
                m_label_matrix[index] = 0; // Reset the label back to 0
                ++num_border_pixels;
                continue;
            }

            // std::nullopt => not in image, treat as border
            std::optional<Point2D> left = getAdjacentPixel(Point2D{x, y},
                                                           Direction::LEFT);
            std::optional<Point2D> up = getAdjacentPixel(Point2D{x, y},
                                                         Direction::UP);

            uint32_t label_left = 0;
            uint32_t label_up = 0;

            Color color_left = BORDER_COLOR;
            Color color_up = BORDER_COLOR;

            // Get the label and color of adjacent pixels that we have already
            //  visited
            if(left) {
                std::tie(label_left, color_left) = getLabelAndColor(*left, color);
            }

            if(up) {
                std::tie(label_up, color_up) = getLabelAndColor(*up, color);
            }

            // Compare the color of the adjacent pixels to ourself
            // getLabelAndColor will auto-convert all colors to BORDER_COLOR if
            //  the color does not match the current one, so there is no need
            //  to check for that here
            if(color_left != BORDER_COLOR) {
                label = label_left;
            }

            if(color_up != BORDER_COLOR) {
                // If we have already chosen an adjacent label
                if(label != next_label) {
                    // If the adjacent label does not match, then pick the
                    //   smaller one and mark the larger one as a child
                    if(label != label_up) {
                        uint32_t smaller = std::min(label, label_up);
                        uint32_t larger = std::max(label, label_up);

                        label = getRootLabel(smaller);
                        // Mark who the parent of the label is
                        // TODO: Do we have to worry about if the label already has a parent?
                        m_label_parents[larger] = smaller;
                    }
                } else {
                    label = label_up;
                }
            }

            if(prog_opts.verbose)
                writeDebug<false>("Pixel ", Point2D{x, y}, " [", color, "] => ",
                                  std::to_string(label));

            // Only increment to the next label if we actually used this one
            if(label == next_label) {
                ++next_label;
            }

            // Only write the color if we are expected to output this stage or
            //  if we are expected to display the stage graphically
            if((prog_opts.output_stages || !prog_opts.no_gui) && m_label_to_color.count(label) == 0)
                m_label_to_color[label] = (label == 0 ? BORDER_COLOR : generateUniqueColor(ProvinceType::UNKNOWN));

            worker.writeDebugColor(x, y, m_label_to_color[label]);
        }

        worker.updateCallback({0, y, width, 1});
    }

    deleteInfoLine();

    return num_border_pixels;
}

/**
 * @brief Performs the second pass of the CCL algorithm
 *
 * @param label_to_shapeidx A mapping which maps every label to the index of the
 *                          shape it will be a part of in the returned list
 *
 * @return A list of all detected shapes
 */
auto MapNormalizer::ShapeFinder::pass2(LabelShapeIdxMap& label_to_shapeidx)
    -> PolygonList&
{
    uint32_t width = m_image->info_header.width;
    uint32_t height = m_image->info_header.height;

    m_shapes.clear();

    auto& worker = GraphicsWorker::getInstance();

    if(!prog_opts.quiet)
        writeStdout("Performing Pass #2 of CCL.");

    for(uint32_t y = 0; y < height; ++y) {
        for(uint32_t x = 0; x < width; ++x) {
            if(m_do_estop) {
                return m_shapes;
            }

            uint32_t index = xyToIndex(m_image, x, y);
            uint32_t& label = m_label_matrix[index];
            Color color = getColorAt(m_image, x, y);
            Point2D point{x, y};

            if(color == BORDER_COLOR) {
                m_border_pixels.push_back(Pixel{ point, color });
                continue;
            }

            // Will return itself if this label is already a root
            label = getRootLabel(label);

            worker.writeDebugColor(x, y, m_label_to_color[label]);

            buildShape(label, Pixel{ point, color }, m_shapes, label_to_shapeidx);
        }

        worker.updateCallback({0, y, width, 1});
    }

    if(!prog_opts.quiet)
        writeStdout("Generated ", m_shapes.size(), " shapes.");

    return m_shapes;
}

/**
 * @brief Merges all border pixels into the nearest shapes.
 *
 * @param shapes The list of shapes to merge border pixels into
 * @param label_to_shapeidx A mapping of which labels correspond to which shapes
 *                          in the provided list
 *
 * @return true if all borders were able to be successfully merged, false
 *         otherwise.
 */
bool MapNormalizer::ShapeFinder::mergeBorders(PolygonList& shapes,
                                              const LabelShapeIdxMap& label_to_shapeidx)
{
    uint32_t width = m_image->info_header.width;
    uint32_t height = m_image->info_header.height;

    auto& worker = GraphicsWorker::getInstance();

    if(!prog_opts.quiet)
        writeStdout("Performing Pass #3 of CCL.");

    for(const Pixel& pixel : m_border_pixels) {
        if(m_do_estop) {
            return false;
        }

        auto&& [x, y] = pixel.point;
        const Point2D& point = pixel.point;

        // Copy it because we will need to change its value later
        Point2D merge_with;

        // Merge with the closest shape
        // First, check anything to our upper-left, as those will be
        //  non-borders if they exist
        std::optional<Point2D> opt_adjacent;

        if(opt_adjacent = getAdjacentPixel(point, Direction::LEFT);
           opt_adjacent)
        {
            merge_with = *opt_adjacent;
        } else if(opt_adjacent = getAdjacentPixel(point, Direction::UP);
                  opt_adjacent)
        {
            merge_with = *opt_adjacent;
        } else {
            bool found = false;
            // If that fails, start walking left->right, top->bottom for the
            //  first pixel that is not a border and merge with that
            for(uint32_t y2 = y; !found && y2 < height; ++y2) {
                for(uint32_t x2 = x; !found && x2 < width; ++x2) {
                    if(m_do_estop) {
                        return false;
                    }

                    // Skip any further border pixels, then will be
                    //  picked up by the earlier adjacency checks
                    if(getColorAt(m_image, x2, y2) == BORDER_COLOR) {
                        continue;
                    } else {
                        merge_with = Point2D{ x2, y2 };
                        found = true;
                    }
                }
            }

            // If we _still_ have a border color, then that means we are
            //  in a worst case scenario of the entire m_image being
            //  (0,0,0)
            if(!found) {
                writeError("No further color pixels found from ", point,
                           ". Terminating now! Check your input m_image!");
                return false;
            }
        }

        uint32_t index = xyToIndex(m_image, merge_with.x, merge_with.y);
        uint32_t label = m_label_matrix[index];

        Polygon& shape = shapes[label_to_shapeidx.at(label)];

        addPixelToShape(shape, pixel);

        m_label_matrix[xyToIndex(m_image, x, y)] = label;

        worker.writeDebugColor(x, y, shape.unique_color);
    }

    worker.updateCallback({0, 0, width, height});

    return true;
}

/**
 * @brief Finds every shape in the image
 * @details Performs a 2-pass implementation of the Connected-Component Labeling
 *          (CCL) algorithm. The particular is adapted from the example here:
 *          https://www.aishack.in/tutorials/labelling-connected-components-example/
 *
 * @return A list of every shape in the image.
 */
const MapNormalizer::PolygonList& MapNormalizer::ShapeFinder::findAllShapes() {
    auto& worker = GraphicsWorker::getInstance();

    m_stage = Stage::PASS1;

    // Do pass 1, and reserve enough space in the m_border_pixels vector for all
    //   border pixels in the image
    uint32_t num_border_pixels = pass1();
    if(m_do_estop) {
        m_do_estop = false;
        m_shapes.clear();
        return m_shapes;
    }

    m_border_pixels.reserve(num_border_pixels);

    m_stage = Stage::OUTPUT_PASS1;

    if(prog_opts.output_stages) {
        m_label_to_color[0] = BORDER_COLOR;
        worker.updateCallback({0, 0, 0, 0});
        outputStage("labels1.bmp");
        if(m_do_estop) {
            m_do_estop = false;
            m_shapes.clear();
            return m_shapes;
        }
    }

    LabelShapeIdxMap label_to_shapeidx;

    // Make sure that any unique colors consumed will go back to the beginning
    resetUniqueColorGenerator();

    m_stage = Stage::PASS2;
    // Do pass 1, we now have all of the shapes in the image, though there are
    //  still the border pixels left over to deal with
    pass2(label_to_shapeidx);
    if(m_do_estop) {
        m_do_estop = false;
        m_shapes.clear();
        return m_shapes;
    }

    m_stage = Stage::OUTPUT_PASS2;
    if(prog_opts.output_stages) {
        worker.updateCallback({0, 0, 0, 0});
        outputStage("labels2.bmp");
        if(m_do_estop) {
            m_do_estop = false;
            m_shapes.clear();
            return m_shapes;
        }
    }

    if(m_do_estop) {
        m_do_estop = false;
        m_shapes.clear();
        return m_shapes;
    }

    // Again, we want to end this function by not consuming any unique colors
    resetUniqueColorGenerator();

    m_stage = Stage::MERGE_BORDERS;
    // Merge all of the border pixels together into surrounding shapes
    //  If this fails, then we return an empty-list of shapes to denote failure
    if(!mergeBorders(m_shapes, label_to_shapeidx) || m_do_estop) {
        m_shapes.clear();
        return m_shapes;
    }

    m_stage = Stage::ERROR_CHECK;
    // Perform error checking. This doesn't actually cause us to fail, just spit
    //   out warnings about the input image (as there isn't much for us to do to
    //   fix any errors ourselves
    errorCheckAllShapes(m_shapes);

    m_do_estop = false;
    m_stage = Stage::DONE;

    return m_shapes;
}

/**
 * @brief Performs error checking on the input image.
 *
 * @param shapes The list of shapes to error check
 *
 * @return The number of problematic shapes detected.
 */
std::optional<uint32_t> MapNormalizer::ShapeFinder::errorCheckAllShapes(const PolygonList& shapes)
{
    uint32_t problematic_shapes = 0;

    // Perform error-checking on shapes
    uint32_t label = 0;
    for(const Polygon& shape : shapes) {
        if(m_do_estop) {
            return std::nullopt;
        }

        ++label;

        // Make sure that the label matrix is updated to reflect the correct
        //  shape labels
        for(auto&& pixel : shape.pixels) {
            auto index = xyToIndex(m_image, pixel.point.x, pixel.point.y);
            m_label_matrix[index] = label;
        }

        // Check for minimum province size.
        //  See: https://hoi4.paradoxwikis.com/Map_modding
        if(shape.pixels.size() <= MIN_SHAPE_SIZE) {
            writeWarning("Shape ", label, " has only ",
                         shape.pixels.size(),
                         " pixels. All provinces are required to have more than ",
                         MIN_SHAPE_SIZE,
                         " pixels. See: https://hoi4.paradoxwikis.com/Map_modding");
            ++problematic_shapes;
        }

        //  Check to make sure bounding boxes aren't too large
        if(auto [width, height] = calcShapeDims(shape);
           isShapeTooLarge(width, height, m_image))
        {
            writeWarning("Shape #", label, " has a bounding box of size ",
                         Point2D{width, height},
                         ". One of these is larger than the allowed ratio of 1/8 * (",
                         m_image->info_header.width, ',', m_image->info_header.height,
                         ") => (", (m_image->info_header.width / 8.0f), ',',
                                   (m_image->info_header.height / 8.0f),
                         "). Check the province borders. Bounds are: ",
                         shape.bottom_left, " to ", shape.top_right);
        }
    }

    if(problematic_shapes == 0) {
        return std::nullopt;
    } else {
        return problematic_shapes;
    }
}

/**
 * @brief Outputs the current stage of labels as an image
 *
 * @param filename The filename to output the stage to.
 */
void MapNormalizer::ShapeFinder::outputStage(const std::string& filename) {
    unsigned char* label_data = new unsigned char[m_label_matrix_size * 3];

    for(uint32_t i = 0; i < m_label_matrix_size; ++i) {
        uint32_t label = m_label_matrix[i];
        const MapNormalizer::Color& c = m_label_to_color[label];
        label_data[i * 3] = c.b;
        label_data[(i * 3) + 1] = c.g;
        label_data[(i * 3) + 2] = c.r;
    }

    MapNormalizer::writeBMP(filename, label_data, m_image->info_header.width,
                            m_image->info_header.height);

    delete[] label_data;
}

/**
 * @brief Gets the label and the color for the given point.
 *
 * @param point The point to get the color and label for.
 * @param color The current color to compare the gotten color against
 *
 * @return A pair containing both the label and the color
 */
auto MapNormalizer::ShapeFinder::getLabelAndColor(const Point2D& point,
                                                  const Color& color)
    -> std::pair<uint32_t, Color>
{
    uint32_t label = m_label_matrix[xyToIndex(m_image, point.x, point.y)];
    Color color_at = getColorAt(m_image, point.x, point.y);

    if(color_at != BORDER_COLOR && color_at != color) {
        writeWarning("Multiple colors found in shape! See pixel at ", point);

        // Set to the default values
        label = 0;
        color_at = BORDER_COLOR;
    }

    return {label, color_at};
}

/**
 * @brief Gets the root label for the given label
 *
 * @param label The label to get the root for
 *
 * @return The root of label
 */
uint32_t MapNormalizer::ShapeFinder::getRootLabel(uint32_t label) {
    uint32_t root = label;

    while(m_label_parents.count(root) != 0) {
        root = m_label_parents.at(root);
    }

    return root;
}

/**
 * @brief Gets a pixel adjacent to point
 *
 * @param point The point to get an adjacent pixel for.
 * @param dir1 The direction.
 *
 * @return The point adjacent to 'point', std::nullopt if there is no pixel
 *         adjacent to 'point' in the directions specified
 */
auto MapNormalizer::ShapeFinder::getAdjacentPixel(const Point2D& point,
                                                  Direction dir1) const
    -> std::optional<Point2D>
{
    Point2D adjacent = point;

    switch(dir1) {
        case Direction::LEFT:
            adjacent.x = point.x - 1;
            break;
        case Direction::RIGHT:
            adjacent.x = point.x + 1;
            break;
        case Direction::UP:
            adjacent.y = point.y - 1;
            break;
        case Direction::DOWN:
            adjacent.y = point.y + 1;
            break;
    }

    if(isInImage(m_image, adjacent.x, adjacent.y) &&
       getColorAt(m_image, adjacent.x, adjacent.y) != BORDER_COLOR)
    {
        return adjacent;
    } else {
        return std::nullopt;
    }
}

/**
 * @brief Builds a shape up.
 *
 * @param label The label of the shape being built
 * @param pixel The pixel to add to a shape
 * @param shapes The list of shapes
 * @param label_to_shapeidx The mapping of labels to their corresponding shapes
 */
void MapNormalizer::ShapeFinder::buildShape(uint32_t label, const Pixel& pixel,
                                            PolygonList& shapes,
                                            LabelShapeIdxMap& label_to_shapeidx)
{
    uint32_t shapeidx = -1;

    // Do we have an entry for this label yet?
    if(label_to_shapeidx.count(label) == 0) {
        label_to_shapeidx[label] = shapes.size();

        // Create a new shape
        auto prov_type = getProvinceType(pixel.color);
        auto unique_color = generateUniqueColor(prov_type);

        shapes.push_back(Polygon{
            { },
            pixel.color,
            unique_color,
            { 0, 0 }, { 0, 0 }
        });
    }

    shapeidx = label_to_shapeidx[label];

    addPixelToShape(shapes.at(shapeidx), pixel);
}

void MapNormalizer::ShapeFinder::estop() {
    m_do_estop = true;
}

MapNormalizer::ShapeFinder::Stage MapNormalizer::ShapeFinder::getStage() const {
    return m_stage;
}

uint32_t MapNormalizer::ShapeFinder::getLabelMatrixSize() {
    return m_label_matrix_size;
}

uint32_t* MapNormalizer::ShapeFinder::getLabelMatrix() {
    return m_label_matrix;
}

auto MapNormalizer::ShapeFinder::getBorderPixels() 
    -> std::vector<Pixel>&
{
    return m_border_pixels;
}

auto MapNormalizer::ShapeFinder::getLabelToColorMap() 
    -> std::map<uint32_t, Color>&
{
    return m_label_to_color;
}

auto MapNormalizer::ShapeFinder::getShapes() -> PolygonList& {
    return m_shapes;
}

const MapNormalizer::BitMap* MapNormalizer::ShapeFinder::getImage() const {
    return m_image;
}

uint32_t MapNormalizer::ShapeFinder::getLabelMatrixSize() const {
    return m_label_matrix_size;
}

const uint32_t* MapNormalizer::ShapeFinder::getLabelMatrix() const {
    return m_label_matrix;
}

auto MapNormalizer::ShapeFinder::getBorderPixels() const
    -> const std::vector<Pixel>&
{
    return m_border_pixels;
}

auto MapNormalizer::ShapeFinder::getLabelToColorMap() const
    -> const std::map<uint32_t, Color>&
{
    return m_label_to_color;
}

auto MapNormalizer::ShapeFinder::getShapes() const -> const PolygonList& {
    return m_shapes;
}

/**
 * @brief Adds a pixel to the given shape.
 * @details As a side-effect, will also expand the shape's bounding box.
 *
 * @param shape The shape to add the pixel to.
 * @param pixel The pixel to add to the shape.
 */
void MapNormalizer::addPixelToShape(Polygon& shape, const Pixel& pixel) {
    shape.pixels.push_back(pixel);

    // We calculate the bounding box of the shape incrementally
    if(pixel.point.x > shape.top_right.x) {
        shape.top_right.x = pixel.point.x;
    } else if(pixel.point.x < shape.bottom_left.x) {
        shape.bottom_left.x = pixel.point.x;
    }

    if(pixel.point.y > shape.top_right.y) {
        shape.top_right.y = pixel.point.y;
    } else if(pixel.point.y < shape.bottom_left.y) {
        shape.bottom_left.y = pixel.point.y;
    }
}

auto MapNormalizer::createPolygonListFromLabels(uint32_t* label_matrix,
                                                uint32_t width, uint32_t height,
                                                const ShapeFinder::LabelToColorMap& label_to_color,
                                                const BitMap* src_image)
    -> PolygonList
{
    uint32_t label_matrix_size = width * height;

    std::map<uint32_t, Polygon> shape_map;
    uint32_t max_label = 1;

    writeDebug("CreatePolygonListFromLabels()");
    for(uint32_t i = 0; i < label_matrix_size; ++i) {
        uint32_t label = label_matrix[i];

        if(label == 0) {
            writeWarning("Found label with value of 0! This should not happen.");
        }

        Color unique_color = BORDER_COLOR;
        if(label_to_color.count(label) != 0) {
            unique_color = label_to_color.at(label);
        }

        Point2D xy{ i / width, i % height };
        Color src_color = getColorAt(src_image, xy.x, xy.y);

        // Find what the largest label is
        max_label = std::max(max_label, label);

        // If we don't already have a shapee for that index, create it
        if(shape_map.count(label) == 0) {
            writeDebug("Found new label ", label);

            shape_map[label] = Polygon {
                {}, 
                src_color,
                unique_color,
                {0, 0},
                {0, 0}
            };
        }

        auto& shape = shape_map.at(label);
        if(shape.color == BORDER_COLOR && src_color != BORDER_COLOR) {
            shape.color = src_color;
        }

        addPixelToShape(shape, Pixel{xy, src_color});
    }

    writeDebug("Converting polygon map into list.");
    // Put the data back into a vector
    PolygonList shapes;
    shapes.reserve(max_label);
    for(uint32_t i = 1; i < max_label; ++i) {
        shapes.push_back(shape_map.at(i));
    }

    writeDebug("Done.");

    return shapes;
}

std::string MapNormalizer::toString(const ShapeFinder::Stage& stage) {
    switch(stage) {
        case ShapeFinder::Stage::START:
            return "Start";
        case ShapeFinder::Stage::PASS1:
            return "Pass 1";
        case ShapeFinder::Stage::OUTPUT_PASS1:
            return "Output Pass 1";
        case ShapeFinder::Stage::PASS2:
            return "Pass 2";
        case ShapeFinder::Stage::OUTPUT_PASS2:
            return "Output Pass 2";
        case ShapeFinder::Stage::MERGE_BORDERS:
            return "Merge Borders";
        case ShapeFinder::Stage::ERROR_CHECK:
            return "Error Checking";
        case ShapeFinder::Stage::DONE:
            return "Done";
    }
}

