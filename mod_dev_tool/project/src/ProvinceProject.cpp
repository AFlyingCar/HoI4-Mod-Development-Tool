
#include "ProvinceProject.h"

#include <fstream>
#include <cerrno>
#include <cstring>

#include "Constants.h"
#include "MapData.h"
#include "Util.h"
#include "StatusCodes.h"
#include "Options.h"

#include "ShapeFinder2.h"

#include "Logger.h"

#include "HoI4Project.h"

HMDT::Project::ProvinceProject::ProvinceProject(IMapProject& parent_project):
    m_parent_project(parent_project),
    m_provinces()
{
}

HMDT::Project::ProvinceProject::~ProvinceProject() {
}

auto HMDT::Project::ProvinceProject::save(const std::filesystem::path& path)
    -> MaybeVoid
{
    if(m_provinces.empty()) {
        WRITE_DEBUG("Nothing to write!");
        return STATUS_SUCCESS;
    }

    auto shapelabels_result = saveShapeLabels(path);
    RETURN_IF_ERROR(shapelabels_result);

    auto provdata_result = saveProvinceData(path);
    RETURN_IF_ERROR(provdata_result);

    return STATUS_SUCCESS;
}

auto HMDT::Project::ProvinceProject::load(const std::filesystem::path& path)
    -> MaybeVoid
{
    auto provdata_result = loadProvinceData(path);
    if(provdata_result.error() == std::errc::no_such_file_or_directory) {
        provdata_result = STATUS_SUCCESS;
    }
    RETURN_IF_ERROR(provdata_result);

    auto shapelabels_result = loadShapeLabels(path);
    RETURN_IF_ERROR(shapelabels_result);

    return STATUS_SUCCESS;
}

void HMDT::Project::ProvinceProject::import(const ShapeFinder& sf, std::shared_ptr<MapData>)
{
    m_provinces = createProvincesFromShapeList(sf.getShapes());

    // Clear out the province preview data
    m_data_cache.clear();

    buildProvinceOutlines();
}

bool HMDT::Project::ProvinceProject::validateData() {
    // We have nothing to really validate here
    return true;
}

HMDT::Project::IProject& HMDT::Project::ProvinceProject::getRootParent() {
    return m_parent_project.getRootParent();
}

HMDT::Project::IMapProject& HMDT::Project::ProvinceProject::getRootMapParent() {
    return m_parent_project.getRootMapParent();
}

auto HMDT::Project::ProvinceProject::getMapData() -> std::shared_ptr<MapData> {
    return m_parent_project.getMapData();
}

auto HMDT::Project::ProvinceProject::getMapData() const
    -> const std::shared_ptr<MapData>
{
    return m_parent_project.getMapData();
}

/**
 * @brief Writes all shape label data to a file.
 *
 * @param root The root where the shape label data should be written to
 *
 * @return True if the data was able to be successfully written, false otherwise.
 */
auto HMDT::Project::ProvinceProject::saveShapeLabels(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / SHAPEDATA_FILENAME;

    // write the shape finder data in a way that we can re-load it later
    if(std::ofstream out(path, std::ios::binary | std::ios::out); out)
    {
        out << SHAPEDATA_MAGIC;

        writeData(out, getMapData()->getWidth(), 
                       getMapData()->getHeight());

        // Write the entire label matrix to the file
        out.write(reinterpret_cast<const char*>(getMapData()->getLabelMatrix().lock().get()),
                  getMapData()->getMatrixSize() * sizeof(uint32_t));
        out << '\0';
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Writes all province data to a .csv file (the same sort of file as
 *        would be loaded by HoI4
 *
 * @param root The root where the csv file should be written to
 *
 * @return True if the file was able to be successfully written, false otherwise.
 */
auto HMDT::Project::ProvinceProject::saveProvinceData(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(std::ofstream out(path); out) {
        // Write one line to the CSV for each province
        for(auto&& province : m_provinces) {
            out << province.id << ';'
                << static_cast<int>(province.unique_color.r) << ';'
                << static_cast<int>(province.unique_color.g) << ';'
                << static_cast<int>(province.unique_color.b) << ';'
                << province.type << ';'
                << (province.coastal ? "true" : "false")
                << ';' << province.terrain << ';'
                << province.continent << ';'
                << province.bounding_box.bottom_left.x << ';'
                << province.bounding_box.bottom_left.y << ';'
                << province.bounding_box.top_right.x << ';'
                << province.bounding_box.top_right.y << ';'
                << province.state
                << std::endl;
        }
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Loads all shape label data out of $root/SHAPEDATA_FILENAME
 *
 * @param root The root where the shapedata is found
 *
 * @return True if the data was able to be loaded successfully, false otherwise
 */
auto HMDT::Project::ProvinceProject::loadShapeLabels(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / SHAPEDATA_FILENAME;

    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("File ", path, " does not exist.");
        return std::make_error_code(std::errc::no_such_file_or_directory);
    } else if(std::ifstream in(path, std::ios::binary | std::ios::in); in) {
        unsigned char magic[4];
        uint32_t width = 0;
        uint32_t height = 0;

        // Read in all header information first, and make sure that we were 
        //  successful
        if(!safeRead(in, &magic, &width, &height)) {
            WRITE_ERROR("Failed to read in header information.");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        // Validate that the width + height for the shape data matches what we
        //   expect.
        if((width * height) != getMapData()->getMatrixSize()) {
            WRITE_ERROR("Loaded shape data size (", (width * height), ") does not match expected matrix size of (", getMapData()->getMatrixSize(), ")");
            RETURN_ERROR(std::make_error_code(std::errc::invalid_argument));
        }

        auto label_matrix = getMapData()->getLabelMatrix().lock();

        if(!safeRead(label_matrix.get(), getMapData()->getMatrixSize() * sizeof(uint32_t), in))
        {
            WRITE_ERROR("Failed to read full label matrix.");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Load all province-level data from $root/PROVINCEDATA_FILENAME, the
 *        same type of .csv file loaded by HoI4.
 *
 * @param root The path to the root where the .csv file is
 *
 * @return True if the file was able to be successfully loaded, false otherwise.
 */
auto HMDT::Project::ProvinceProject::loadProvinceData(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("File ", path, " does not exist.");
        return std::make_error_code(std::errc::no_such_file_or_directory);
    } else if(std::ifstream in(path); in) {
        std::string line;

        // Make sure we don't have any provinces in the list first
        m_provinces.clear();

        // Get every line from the CSV file for parsing
        for(uint32_t line_num = 1; std::getline(in, line); ++line_num) {
            if(line.empty()) continue;

            std::stringstream ss(line);

            Province prov;

            // Attempt to parse the entire CSV line, we expect it to look like:
            //  ID;R;G;B;ProvinceType;IsCoastal;TerrainType;ContinentID;BB.BottomLeft.X;BB.BottomLeft.Y;BB.TopRight.X;BB.TopRight.Y;StateID
            if(!parseValuesSkipMissing<';'>(ss, &prov.id,
                                                &prov.unique_color.r,
                                                &prov.unique_color.g,
                                                &prov.unique_color.b,
                                                &prov.type,
                                                &prov.coastal,
                                                &prov.terrain,
                                                &prov.continent,
                                                &prov.bounding_box.bottom_left.x,
                                                &prov.bounding_box.bottom_left.y,
                                                &prov.bounding_box.top_right.x,
                                                &prov.bounding_box.top_right.y,
                                                &prov.state, true))
            {
                WRITE_ERROR("Failed to parse line #", line_num, ": '", line, "'");
                RETURN_ERROR(std::make_error_code(std::errc::bad_message));
            }

            m_provinces.push_back(prov);
        }

        WRITE_DEBUG("Loaded information for ",
                   m_provinces.size(), " provinces");
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

HMDT::ProvinceList& HMDT::Project::ProvinceProject::getProvinces() {
    return m_provinces;
}

const HMDT::ProvinceList& HMDT::Project::ProvinceProject::getProvinces() const {
    return m_provinces;
}

/**
 * @brief Will build the province preview for the given province. If more than
 *        MAX_CACHED_PROVINCE_PREVIEWS are already stored, then the least
 *        accessed preview will be kicked out of the cache before a new preview
 *        is constructed.
 *
 * @param province_ptr
 */
void HMDT::Project::ProvinceProject::buildProvinceCache(const Province* province_ptr)
{
    const auto& province = *province_ptr;
    auto id = province.id;

    // If there are too many cached provinces, then remove the least accessed
    //  one (which should be the first one in the cache)
    if(m_data_cache.size() > MAX_CACHED_PROVINCE_PREVIEWS) {
        // Do not clear out the first value if it is the one we are trying to
        //  create.
        if(m_data_cache.begin()->first == id) {
            m_data_cache.erase(std::next(m_data_cache.begin()));
        } else {
            m_data_cache.erase(m_data_cache.begin());
        }
    }

    auto& data = m_data_cache[id];

    WRITE_DEBUG("No preview data for province ", id, ". Building...");

    // Some references first, to make the following code easier to read
    //  id also starts at 1, so make sure we offset it down
    auto label_matrix = getMapData()->getLabelMatrix().lock();
    auto iwidth = getMapData()->getWidth();

    auto&& bb = province.bounding_box;
    auto&& [width, height] = calcDims(bb);

    constexpr auto depth = 4;

    // Make sure we 0-initialize the array
    //  We use a depth of 4 since we have RGBA
    data.reset(new unsigned char[width * height * depth]());

    WRITE_DEBUG("Allocated space for ", width * height * depth, " bytes.");
    for(auto x = bb.bottom_left.x; x < bb.top_right.x; ++x) {
        for(auto y = bb.top_right.y; y < bb.bottom_left.y; ++y) {
            // Get the index into the label matrix
            auto lindex = xyToIndex(iwidth, x, y);

            // Offset the x,y so that we get 0-preview width/height
            auto relx = x - bb.bottom_left.x;
            auto rely = y - bb.top_right.y;

            // Index into the cached data
            //  Need a special one since the depth is different from the
            //  stored graphics data
            auto dindex = xyToIndex(width * depth, relx * depth, rely);

            auto label = label_matrix[lindex];

            if(label == id) {
                // ARGB
                *reinterpret_cast<uint32_t*>(&data[dindex]) = PROVINCE_HIGHLIGHT_COLOR;
            }
        }
    }

    WRITE_DEBUG("Done.");

    if(prog_opts.debug) {
        auto path = dynamic_cast<HoI4Project&>(getRootParent()).getMetaRoot() / "debug";
        auto fname = path / (std::string("prov_preview") + std::to_string(id) + ".pam");

        if(!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
        }

        WRITE_DEBUG("Writing province ", width, 'x', height, " (", id, ") to ", fname);

        if(std::ofstream out(fname); out) {
            out << "P7\nWIDTH " << width << "\nHEIGHT " << height << "\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
            for(auto i = 0; i < width * height * 4; i += 4) {
                // Write Alpha first
                out.write(reinterpret_cast<char*>(&data[i + 3]), 1);
                out.write(reinterpret_cast<char*>(&data[i + 1]), 3);
            }
        }
    }
}

void HMDT::Project::ProvinceProject::buildProvinceOutlines() {
    auto prov_outline_data = getMapData()->getProvinceOutlines().lock();
    auto graphics_data = getMapData()->getProvinces().lock();

    auto [width, height] = getMapData()->getDimensions();
    Dimensions dimensions{width, height};

    // Go over the map again and build extra data that depends on the previously
    //  re-built province data
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            auto lindex = xyToIndex(width, x, y);
            auto label = getMapData()->getLabelMatrix().lock()[lindex];
            auto gindex = xyToIndex(width * 4, x * 4, y);

            auto& province = getProvinceForLabel(label);

            // Error check
            if(label <= 0 || label > getProvinces().size()) {
                WRITE_WARN("Label matrix has label ", label,
                             " at position (", x, ',', y, "), which is out of "
                             "the range of valid labels [1,",
                             getProvinces().size(), "]");
                continue;
            }

            // Recalculate adjacencies for this pixel
            auto is_adjacent = ShapeFinder::calculateAdjacency(dimensions,
                                                               graphics_data.get(),
                                                               getMapData()->getLabelMatrix().lock().get(),
                                                               province.adjacent_provinces,
                                                               {x, y});
            // If this pixel is adjacent to any others, then make it visible as
            //  an outline
            if(is_adjacent) {
                prov_outline_data[gindex] = 0xFF;
                prov_outline_data[gindex + 1] = 0xFF;
                prov_outline_data[gindex + 2] = 0xFF;
                prov_outline_data[gindex + 3] = 0xFF;
            }
        }
    }
}

/**
 * @brief Gets province preview data for the given ID
 *
 * @param id
 *
 * @return The preview data, or nullptr if the ID does not exist
 */
auto HMDT::Project::ProvinceProject::getPreviewData(ProvinceID id)
    -> ProvinceDataPtr
{
    if(isValidProvinceLabel(id)) {
        return getPreviewData(&getProvinceForLabel(id));
    }

    return nullptr;
}

/**
 * @brief Gets the preview data for the given province. If no data currently
 *        exists, construct it and cache it.
 *
 * @param province_ptr
 *
 * @return The preview data. This method should never return nullptr
 */
auto HMDT::Project::ProvinceProject::getPreviewData(const Province* province_ptr)
    -> ProvinceDataPtr
{
    const auto& province = *province_ptr;
    auto id = province.id;

    auto data = m_data_cache[id];

    // If there is no cached data for the given province ID, then generate the
    //  data for the preview
    if(data == nullptr) {
        buildProvinceCache(province_ptr);
    }

    // Reset access time, cycles the element to the end of the FIFO queue
    data = m_data_cache[id];
    m_data_cache.erase(id);
    m_data_cache[id] = data;

    return data;
}

