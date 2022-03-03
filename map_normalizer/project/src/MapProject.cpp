
#include "MapProject.h"

#include <fstream>
#include <cstring>
#include <cerrno>

#include "Options.h"
#include "Logger.h"
#include "Constants.h"
#include "Util.h"

#include "ProvinceMapBuilder.h"

#include "HoI4Project.h"

MapNormalizer::Project::MapProject::MapProject(IProject& parent_project):
    m_shape_detection_info(),
    m_continents(),
    m_terrains(getDefaultTerrains()),
    m_states(),
    m_selected_provinces(),
    m_parent_project(parent_project)
{
}

MapNormalizer::Project::MapProject::~MapProject() {
}

/**
 * @brief Saves all map data
 *
 * @param path The root path of all map relatedd data
 *
 * @return True if all data was ablee to be successfully loaded, false otherwise
 */
bool MapNormalizer::Project::MapProject::save(const std::filesystem::path& path,
                                              std::error_code& ec)
{
    if(!std::filesystem::exists(path)) {
        WRITE_DEBUG("Creating directory ", path);
        std::filesystem::create_directory(path);
    }

    if(m_shape_detection_info.provinces.empty()) {
        WRITE_DEBUG("Nothing to write!");
        return true;
    }

    return saveShapeLabels(path, ec) && saveProvinceData(path, ec) &&
           saveContinentData(path, ec);
}

/**
 * @brief Loads all map data
 *
 * @param path The root path of all map related data
 *
 * @return True if all data was able to be successfully loaded, false otherwise
 */
bool MapNormalizer::Project::MapProject::load(const std::filesystem::path& path,
                                              std::error_code& ec)
{
    // If there is no root path for this subproject, then don't bother trying
    //  to load
    if(!std::filesystem::exists(path)) {
        // Do not set an error_code as there was no error, but return false
        //  because we are not actually loading any data
        return false;
    }

    // First we try to load the input map back up, as it holds important info
    //  about the map itself (such as dimensions, the original color value, etc...)
    std::unique_ptr<BitMap> input_image(new BitMap);

    auto& map_data = m_shape_detection_info.map_data;

    auto inputs_root = dynamic_cast<HoI4Project&>(m_parent_project).getInputsRoot();
    auto input_provincemap_path = inputs_root / INPUT_PROVINCEMAP_FILENAME;
    if(!std::filesystem::exists(input_provincemap_path)) {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        WRITE_WARN("Source import image does not exist, unable to finish loading data.");
        return false;
    } else {
        readBMP(input_provincemap_path, input_image.get());

        if(input_image == nullptr) {
            // TODO: We should instead pass ec into readBMP() and let it set ec
            //  to whatever might be appropriate
            ec = std::make_error_code(std::errc::io_error);
            WRITE_WARN("Failed to read imported image.");
            return false;
        }

        auto iwidth = input_image->info_header.width;
        auto iheight = input_image->info_header.height;

        map_data.reset(new MapData(iwidth, iheight));
    }

    // Now load the other related data
    // This data is required
    if(!loadProvinceData(path, ec) || !loadShapeLabels(path, ec)) {
        return false;
    }

    // This data is not required (only fail if loading it failed), not if it 
    //  doesn't exist
    if(!loadContinentData(path, ec) && ec.value() != 0) {
        return false;
    }

    // Rebuild the graphics data
    auto [width, height] = map_data->getDimensions();

    // TODO: Why do we actually have to do this? For some reason everything stops
    //  rendering correctly if we remove this line. Why? What? How?
    auto label_matrix = map_data->getLabelMatrix().lock();
    map_data.reset(new MapData(width, height));
    map_data->setLabelMatrix(label_matrix); // WHY?!!!!

    auto input_data = map_data->getInput().lock();
    auto graphics_data = map_data->getProvinces().lock();

    // Copy the input image's data into the input_data
    std::copy(input_data.get(), input_data.get() + (width * height * 3), input_image->data);

    // Rebuild the map_data array and the adjacency lists
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            // Get the index into the label matrix
            auto lindex = xyToIndex(width, x, y);

            // Get the index into the graphics data
            //  3 == the depth
            auto gindex = xyToIndex(width * 3, x * 3, y);

            auto label = label_matrix[lindex];

            // Error check
            if(label <= 0 || label > m_shape_detection_info.provinces.size()) {
                WRITE_WARN("Label matrix has label ", label,
                             " at position (", x, ',', y, "), which is out of "
                             "the range of valid labels [1,",
                             m_shape_detection_info.provinces.size(), "]");
                continue;
            }

            // Rebuild color data
            auto& province = m_shape_detection_info.provinces[label - 1];

            // Flip the colors from RGB to BGR because BitMap is a bad format
            graphics_data[gindex] = province.unique_color.b;
            graphics_data[gindex + 1] = province.unique_color.g;
            graphics_data[gindex + 2] = province.unique_color.r;
        }
    }

    buildProvinceOutlines();

    return true;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Writes all shape label data to a file.
 *
 * @param root The root where the shape label data should be written to
 *
 * @return True if the data was able to be successfully written, false otherwise.
 */
bool MapNormalizer::Project::MapProject::saveShapeLabels(const std::filesystem::path& root,
                                                         std::error_code& ec)
{
    auto path = root / SHAPEDATA_FILENAME;

    // write the shape finder data in a way that we can re-load it later
    if(std::ofstream out(path, std::ios::binary | std::ios::out); out)
    {
        out << SHAPEDATA_MAGIC;

        writeData(out, m_shape_detection_info.map_data->getWidth(), 
                       m_shape_detection_info.map_data->getHeight());

        // Write the entire label matrix to the file
        out.write(reinterpret_cast<const char*>(m_shape_detection_info.map_data->getLabelMatrix().lock().get()),
                  m_shape_detection_info.label_matrix_size * sizeof(uint32_t));
        out << '\0';
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Writes all province data to a .csv file (the same sort of file as
 *        would be loaded by HoI4
 *
 * @param root The root where the csv file should be written to
 *
 * @return True if the file was able to be successfully written, false otherwise.
 */
bool MapNormalizer::Project::MapProject::saveProvinceData(const std::filesystem::path& root,
                                                          std::error_code& ec)
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(std::ofstream out(path); out) {
        // Write one line to the CSV for each province
        for(auto&& province : m_shape_detection_info.provinces) {
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
                << province.bounding_box.top_right.y
                << std::endl;
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Writes all continent data to root/$CONTINENTDATA_FILENAME
 *
 * @param root The root where all continent data should go
 * @param ec The error code
 *
 * @return True if continent data was successfully loaded, false otherwise
 */
bool MapNormalizer::Project::MapProject::saveContinentData(const std::filesystem::path& root,
                                                           std::error_code& ec)
{
    auto path = root / CONTINENTDATA_FILENAME;

    // Try to open the continent file for reading.
    if(std::ofstream out(path); out) {
        for(auto&& continent : m_continents) {
            out << continent << '\n';
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Loads all shape label data out of $root/SHAPEDATA_FILENAME
 *
 * @param root The root where the shapedata is found
 *
 * @return True if the data was able to be loaded successfully, false otherwise
 */
bool MapNormalizer::Project::MapProject::loadShapeLabels(const std::filesystem::path& root,
                                                         std::error_code& ec)
{
    auto path = root / SHAPEDATA_FILENAME;

    if(!std::filesystem::exists(path)) {
        WRITE_WARN("File ", path, " does not exist.");
        return false;
    } else if(std::ifstream in(path, std::ios::binary | std::ios::in); in) {
        unsigned char magic[4];
        uint32_t width = 0;
        uint32_t height = 0;

        // Read in all header information first, and make sure that we were 
        //  successful
        if(!safeRead(in, &magic, &width, &height)) {
            ec = std::error_code(static_cast<int>(errno), std::generic_category());
            WRITE_ERROR("Failed to read in header information. Reason: ", std::strerror(errno));
            return false;
        }

        auto& label_matrix_size = m_shape_detection_info.label_matrix_size;

        label_matrix_size = width * height;
        m_shape_detection_info.map_data->setLabelMatrix(new uint32_t[label_matrix_size]);

        auto label_matrix = m_shape_detection_info.map_data->getLabelMatrix().lock();

        if(!safeRead(label_matrix.get(), label_matrix_size * sizeof(uint32_t), in)) {
            ec = std::error_code(static_cast<int>(errno), std::generic_category());
            WRITE_ERROR("Failed to read full label matrix. Reason: ", std::strerror(errno));
            return false;
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Load all province-level data from $root/PROVINCEDATA_FILENAME, the
 *        same type of .csv file loaded by HoI4.
 *
 * @param root The path to the root where the .csv file is
 *
 * @return True if the file was able to be successfully loaded, false otherwise.
 */
bool MapNormalizer::Project::MapProject::loadProvinceData(const std::filesystem::path& root,
                                                          std::error_code& ec)
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(!std::filesystem::exists(path, ec)) {
        WRITE_WARN("File ", path, " does not exist.");
        return false;
    } else if(std::ifstream in(path); in) {
        std::string line;

        // Make sure we don't have any provinces in the list first
        m_shape_detection_info.provinces.clear();

        // Get every line from the CSV file for parsing
        for(uint32_t line_num = 1; std::getline(in, line); ++line_num) {
            if(line.empty()) continue;

            std::stringstream ss(line);

            Province prov;

            // Attempt to parse the entire CSV line, we expect it to look like:
            //  ID;R;G;B;ProvinceType;IsCoastal;TerrainType;ContinentID;BB.BottomLeft.X;BB.BottomLeft.Y;BB.TopRight.X;BB.TopRight.Y
            if(!parseValues<';'>(ss, prov.id, prov.unique_color.r,
                                              prov.unique_color.g,
                                              prov.unique_color.b,
                                     prov.type, prov.coastal, prov.terrain,
                                     prov.continent,
                                     prov.bounding_box.bottom_left.x,
                                     prov.bounding_box.bottom_left.y,
                                     prov.bounding_box.top_right.x,
                                     prov.bounding_box.top_right.y))
            {
                ec = std::make_error_code(std::errc::bad_message);
                WRITE_ERROR("Failed to parse line #", line_num, ": '", line, "'");
                return false;
            }

            m_shape_detection_info.provinces.push_back(prov);
        }

        WRITE_DEBUG("Loaded information for ",
                   m_shape_detection_info.provinces.size(), " provinces");
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Loads all continent data from a file
 *
 * @param root The root where the continent data file should be found
 * @param ec The error code
 *
 * @return True if data was loaded correctly, false otherwise
 */
bool MapNormalizer::Project::MapProject::loadContinentData(const std::filesystem::path& root,
                                                           std::error_code& ec)
{
    auto path = root / CONTINENTDATA_FILENAME;

    // If the file doesn't exist, then return false (we didn't actually load it
    //  after all), but don't set the error code as it is expected that the
    //  file may not exist
    if(!std::filesystem::exists(path)) {
        WRITE_WARN("No data to load! No continents currently exist!");
        return false;
    }

    if(std::ifstream in(path); in) {
        std::string line;
        while(std::getline(in, line)) {
            if(line.empty()) continue;

            m_continents.insert(line);
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Loads data out of a ShapeFinder. We invalidate the original ShapeFinder
 *        as we want to take ownership of all pointers it holds
 *
 * @param shape_finder The ShapeFinder to load data from
 */
void MapNormalizer::Project::MapProject::setShapeFinder(ShapeFinder&& shape_finder)
{
    // We want to take ownership of all the internal data here
    //  TODO: Do we actually _need_ to do this?
    ShapeFinder sf(std::move(shape_finder));

    m_shape_detection_info.provinces = createProvincesFromShapeList(sf.getShapes());
    m_shape_detection_info.map_data->setLabelMatrix(sf.getLabelMatrix());
    m_shape_detection_info.label_matrix_size = sf.getLabelMatrixSize();
    m_shape_detection_info.map_data.reset();

    // Clear out which province is selected
    m_data_cache.clear();
}

void MapNormalizer::Project::MapProject::setGraphicsData(std::shared_ptr<MapData> map_data) {
    m_shape_detection_info.map_data = map_data;

    buildProvinceOutlines();
}

auto MapNormalizer::Project::MapProject::getMapData()
    -> std::shared_ptr<MapData>
{
    return m_shape_detection_info.map_data;
}

auto MapNormalizer::Project::MapProject::getMapData() const
    -> const std::shared_ptr<MapData>
{
    return m_shape_detection_info.map_data;
}

const uint32_t* MapNormalizer::Project::MapProject::getLabelMatrix() const {
    return m_shape_detection_info.map_data->getLabelMatrix().lock().get();
}

void MapNormalizer::Project::MapProject::selectProvince(uint32_t label) {
    m_selected_provinces = {label};
}

void MapNormalizer::Project::MapProject::addProvinceSelection(uint32_t label) {
    m_selected_provinces.insert(label);
}

void MapNormalizer::Project::MapProject::removeProvinceSelection(uint32_t label)
{
    m_selected_provinces.erase(label);
}

void MapNormalizer::Project::MapProject::clearProvinceSelection() {
    m_selected_provinces.clear();
}

auto MapNormalizer::Project::MapProject::getProvinceForLabel(uint32_t label) const
    -> const Province&
{
    return m_shape_detection_info.provinces.at(label);
}

auto MapNormalizer::Project::MapProject::getProvinceForLabel(uint32_t label)
    -> Province&
{
    return m_shape_detection_info.provinces.at(label);
}

/**
 * @brief Will return the currently selected provinces.
 *
 * @return The currently selected provinces.
 */
auto MapNormalizer::Project::MapProject::getSelectedProvinces() const
    -> RefVector<const Province>
{
    RefVector<const Province> provinces;
    std::transform(m_selected_provinces.begin(), m_selected_provinces.end(),
                   std::back_inserter(provinces),
                   [this](uint32_t prov_id) {
                       return std::ref(m_shape_detection_info.provinces.at(prov_id));
                   });
    return provinces;
}

/**
 * @brief Will return the currently selected provinces.
 *
 * @return The currently selected provinces.
 */
auto MapNormalizer::Project::MapProject::getSelectedProvinces()
    -> RefVector<Province>
{
    RefVector<Province> provinces;
    std::transform(m_selected_provinces.begin(), m_selected_provinces.end(),
                   std::back_inserter(provinces),
                   [this](uint32_t prov_id) {
                       return std::ref(m_shape_detection_info.provinces.at(prov_id));
                   });
    return provinces;
}

auto MapNormalizer::Project::MapProject::getSelectedProvinceLabels() const
    -> const std::set<uint32_t>&
{
    return m_selected_provinces;
}

const std::set<std::string>& MapNormalizer::Project::MapProject::getContinentList() const
{
    return m_continents;
}

auto MapNormalizer::Project::MapProject::getTerrains() const
    -> const std::vector<Terrain>&
{
    return m_terrains;
}

void MapNormalizer::Project::MapProject::addNewContinent(const std::string& continent)
{
    m_continents.insert(continent);
}

void MapNormalizer::Project::MapProject::removeContinent(const std::string& continent)
{
    m_continents.erase(continent);
}

/**
 * @brief Creates a new state composed of all provinces in province_ids.
 * @details The provinces detailed in province_ids will get removed from their
 *          original states. Note however that the original states will still
 *          exist.
 *
 * @param province_ids The list of provinces to add to the new state.
 */
void MapNormalizer::Project::MapProject::addNewState(const std::vector<uint32_t>& province_ids)
{
    RefVector<Province> provinces;
    std::transform(province_ids.begin(), province_ids.end(),
                   std::back_inserter(provinces),
                   [this](uint32_t prov_id) -> std::reference_wrapper<Province> {
                       return std::ref(m_shape_detection_info.provinces.at(prov_id));
                   });

    StateID id = m_states.size();
    if(m_available_state_ids.size() > 0) {
        id = m_available_state_ids.front();
        m_available_state_ids.pop();
    }

    // Make sure that the provinces are decoupled from their original state
    for(auto&& prov : provinces) {
        removeProvinceFromState(prov);
        prov.get().state = id;
    }

    m_states[id] = State {
        id,
        "", /* name */
        0, /* manpower */
        "", /* category */
        province_ids
    };
}

/**
 * @brief Deletes the state at id
 *
 * @param id The ID of the state to delete
 */
void MapNormalizer::Project::MapProject::removeState(StateID id) {
    m_available_state_ids.push(id);
    m_states.erase(id);
}

/**
 * @brief Moves a province to another state.
 *
 * @param prov_id The ID of the province to move.
 * @param state_id The ID of the state to move the province to.
 */
void MapNormalizer::Project::MapProject::moveProvinceToState(uint32_t prov_id,
                                                             StateID state_id)
{
    moveProvinceToState(m_shape_detection_info.provinces.at(prov_id), state_id);
}

/**
 * @brief Moves a province to another state.
 *
 * @param province The province to move.
 * @param state_id The ID of the state to move the province to.
 */
void MapNormalizer::Project::MapProject::moveProvinceToState(Province& province,
                                                             StateID state_id)
{
    removeProvinceFromState(province);
    province.state = state_id;
    m_states[state_id].provinces.push_back(province.id);
}

/**
 * @brief Removes a province from its state.
 *
 * @param province The province to remove.
 */
void MapNormalizer::Project::MapProject::removeProvinceFromState(Province& province)
{
    // Remove from its old state
    if(auto prov_state_id = province.state; m_states.count(prov_state_id) != 0)
    {
        auto& state_provinces = m_states[prov_state_id].provinces;

        state_provinces.erase(std::find(state_provinces.begin(),
                                        state_provinces.end(),
                                        province.id));
    }
    province.state = -1;
}

/**
 * @brief Gets province preview data for the given ID
 *
 * @param id
 *
 * @return The preview data, or nullptr if the ID does not exist
 */
auto MapNormalizer::Project::MapProject::getPreviewData(ProvinceID id)
    -> ProvinceDataPtr
{
    if(id - 1 < m_shape_detection_info.provinces.size()) {
        return getPreviewData(&m_shape_detection_info.provinces.at(id - 1));
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
auto MapNormalizer::Project::MapProject::getPreviewData(const Province* province_ptr)
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

auto MapNormalizer::Project::MapProject::getProvinces() const
    -> const ProvinceList&
{
    return m_shape_detection_info.provinces;
}

auto MapNormalizer::Project::MapProject::getProvinces() -> ProvinceList& {
    return m_shape_detection_info.provinces;
}

/**
 * @brief Will build the province preview for the given province. If more than
 *        MAX_CACHED_PROVINCE_PREVIEWS are already stored, then the least
 *        accessed preview will be kicked out of the cache before a new preview
 *        is constructed.
 *
 * @param province_ptr
 */
void MapNormalizer::Project::MapProject::buildProvinceCache(const Province* province_ptr)
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
    auto label_matrix = m_shape_detection_info.map_data->getLabelMatrix().lock();
    auto iwidth = m_shape_detection_info.map_data->getWidth();

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
        auto path = dynamic_cast<HoI4Project&>(m_parent_project).getMetaRoot() / "debug";
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

void MapNormalizer::Project::MapProject::buildProvinceOutlines() {
    auto map_data = m_shape_detection_info.map_data;
    auto prov_outline_data = map_data->getProvinceOutlines().lock();
    auto graphics_data = map_data->getProvinces().lock();

    auto [width, height] = map_data->getDimensions();
    Dimensions dimensions{width, height};

    // Go over the map again and build extra data that depends on the previously
    //  re-built province data
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            auto lindex = xyToIndex(width, x, y);
            auto label = m_shape_detection_info.map_data->getLabelMatrix().lock()[lindex];
            auto gindex = xyToIndex(width * 4, x * 4, y);

            auto& province = m_shape_detection_info.provinces[label - 1];

            // Error check
            if(label <= 0 || label > m_shape_detection_info.provinces.size()) {
                WRITE_WARN("Label matrix has label ", label,
                             " at position (", x, ',', y, "), which is out of "
                             "the range of valid labels [1,",
                             m_shape_detection_info.provinces.size(), "]");
                continue;
            }

            // Recalculate adjacencies for this pixel
            auto is_adjacent = ShapeFinder::calculateAdjacency(dimensions,
                                                               graphics_data.get(),
                                                               m_shape_detection_info.map_data->getLabelMatrix().lock().get(),
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

