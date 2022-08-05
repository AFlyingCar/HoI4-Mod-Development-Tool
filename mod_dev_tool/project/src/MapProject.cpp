
#include "MapProject.h"

#include <fstream>
#include <cstring>
#include <cerrno>

#include "Options.h"
#include "Logger.h"
#include "Constants.h"
#include "Util.h"
#include "StatusCodes.h"

#include "ProvinceMapBuilder.h"

#include "HoI4Project.h"

HMDT::Project::MapProject::MapProject(IProject& parent_project):
    m_provinces_project(*this),
    m_state_project(*this),
    m_continent_project(*this),
    m_map_data(new MapData),
    m_terrains(getDefaultTerrains()),
    m_parent_project(parent_project)
{
}

HMDT::Project::MapProject::~MapProject() {
}

/**
 * @brief Saves all map data
 *
 * @param path The root path of all map relatedd data
 *
 * @return True if all data was ablee to be successfully loaded, false otherwise
 */
auto HMDT::Project::MapProject::save(const std::filesystem::path& path)
    -> MaybeVoid
{
    if(!std::filesystem::exists(path)) {
        WRITE_DEBUG("Creating directory ", path);
        std::filesystem::create_directory(path);
    }

    auto provinces_result = m_provinces_project.save(path);
    RETURN_IF_ERROR(provinces_result);

    auto continents_result = m_continent_project.save(path);
    RETURN_IF_ERROR(continents_result);

    auto states_result = m_state_project.save(path);
    RETURN_IF_ERROR(states_result);

    return STATUS_SUCCESS;
}

/**
 * @brief Loads all map data
 *
 * @param path The root path of all map related data
 *
 * @return True if all data was able to be successfully loaded, false otherwise
 */
auto HMDT::Project::MapProject::load(const std::filesystem::path& path)
    -> MaybeVoid
{
    // If there is no root path for this subproject, then don't bother trying
    //  to load
    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("File ", path, " does not exist.");
        return std::make_error_code(std::errc::no_such_file_or_directory);
    }

    // First we try to load the input map back up, as it holds important info
    //  about the map itself (such as dimensions, the original color value, etc...)
    std::unique_ptr<BitMap> input_image(new BitMap);

    auto inputs_root = dynamic_cast<HoI4Project&>(m_parent_project).getInputsRoot();
    auto input_provincemap_path = inputs_root / INPUT_PROVINCEMAP_FILENAME;
    if(!std::filesystem::exists(input_provincemap_path)) {
        WRITE_WARN("Source import image does not exist, unable to finish loading data.");
        RETURN_ERROR(std::make_error_code(std::errc::no_such_file_or_directory));
    } else {
        readBMP(input_provincemap_path, input_image.get());

        if(input_image == nullptr) {
            // TODO: We should instead pass ec into readBMP() and let it set ec
            //  to whatever might be appropriate
            WRITE_WARN("Failed to read imported image.");
            RETURN_ERROR(std::make_error_code(std::errc::io_error));
        }

        auto iwidth = input_image->info_header.width;
        auto iheight = input_image->info_header.height;

        // Do a placement new so we keep the same memory location but update all
        //  of the data inside the shared MapData instead, so that all
        //  references are also updated too
        m_map_data->~MapData();
        new (m_map_data.get()) MapData(iwidth, iheight);
    }

    // Now load the other related data
    // This data is required
    RETURN_IF_ERROR(m_provinces_project.load(path));

    // This data is not required (only fail if loading it failed), not if it 
    //  doesn't exist
    if(auto result = m_continent_project.load(path);
            result.error() != std::errc::no_such_file_or_directory)
    {
        RETURN_IF_ERROR(result);
    }

    if(auto result = m_state_project.load(path);
            result.error() != std::errc::no_such_file_or_directory)
    {
        RETURN_IF_ERROR(result);
    }

    // Rebuild the graphics data
    auto [width, height] = m_map_data->getDimensions();

    auto label_matrix = m_map_data->getLabelMatrix().lock();

    auto input_data = m_map_data->getInput().lock();
    auto graphics_data = m_map_data->getProvinces().lock();

    // Copy the input image's data into the input_data
    std::copy(input_data.get(), input_data.get() + m_map_data->getInputSize(), input_image->data);

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
            if(label <= 0 || label > m_provinces_project.getProvinces().size()) {
                WRITE_WARN("Label matrix has label ", label,
                             " at position (", x, ',', y, "), which is out of "
                             "the range of valid labels [1,",
                             m_provinces_project.getProvinces().size(), "]");
                continue;
            }

            // Rebuild color data
            auto& province = getProvinceForLabel(label);

            // Flip the colors from RGB to BGR because BitMap is a bad format
            graphics_data[gindex] = province.unique_color.b;
            graphics_data[gindex + 1] = province.unique_color.g;
            graphics_data[gindex + 2] = province.unique_color.r;
        }
    }

    buildProvinceOutlines();

    RETURN_ERROR_IF(!validateData(), STATUS_PROJECT_VALIDATION_FAILED);

    return STATUS_SUCCESS;
}

bool HMDT::Project::MapProject::validateData() {
    WRITE_DEBUG("Validating all project data.");

    bool success = true;

    success = success && m_provinces_project.validateData();

    for(auto&& province : m_provinces_project.getProvinces()) {
        auto result = m_state_project.validateProvinceStateID(province.state, province.id);

        if(IS_FAILURE(result)) {
            if(prog_opts.fix_warnings_on_load) {
                WRITE_INFO("Attempting to fix warnings...");
                if(result.error() == STATUS_PROVINCE_INVALID_STATE_ID) {
                    WRITE_INFO("Setting province state ID to -1.");
                    province.state = -1;
                } else if(result.error() == STATUS_PROVINCE_NOT_IN_STATE) {
                    m_state_project.getStateForID(province.state).andThen([&](auto prov_state_ref)
                    {
                        auto& prov_state = prov_state_ref.get();

                        WRITE_INFO("Adding province ", province.id, " to state ", prov_state.id);
                        prov_state.provinces.push_back(province.id);
                    }).orElse<void>([&province]() {
                        WRITE_ERROR("Unable to remove province ", province.id,
                                    " from its old state ", province.state,
                                    " as that state does not exist. Will"
                                    " attempt to add it to the correct state"
                                    " anyway.");
                    });

                    WRITE_INFO("Searching for any state that currently has this province...");
                    std::vector<ProvinceID>::const_iterator province_it;
                    auto it = std::find_if(getStates().begin(),
                                           getStates().end(),
                                           [&province, &province_it](auto& id_state_pair)
                                           {
                                               return (province_it = std::find(id_state_pair.second.provinces.begin(),
                                                                               id_state_pair.second.provinces.end(),
                                                                               province.id)) != id_state_pair.second.provinces.end();
                                           });

                    if(it == getStates().end()) {
                        WRITE_INFO("No states found containing this province.");
                    } else {
                        WRITE_INFO("Found state ", it->second.id, " that contains province ", province.id, ". Removing the province from the state.");
                        State& state = m_state_project.getStateForIterator(it);
                        state.provinces.erase(province_it);
                    }
                }
            }

            continue;
        }
    }

    return success;
}

HMDT::Project::IProject& HMDT::Project::MapProject::getRootParent() {
    return m_parent_project.getRootParent();
}

HMDT::Project::IMapProject& HMDT::Project::MapProject::getRootMapParent() {
    return *this;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Loads data out of a ShapeFinder. We invalidate the original ShapeFinder
 *        as we want to take ownership of all pointers it holds
 *
 * @param shape_finder The ShapeFinder to load data from
 */
void HMDT::Project::MapProject::import(const ShapeFinder& sf,
                                       std::shared_ptr<MapData> map_data)
{
    // Do a placement new to make sure that we use the same memory location
    m_map_data->~MapData();
    new (m_map_data.get()) MapData(map_data.get());

    {
        m_provinces_project.import(sf, map_data);

        // Clear out the province preview data
        m_data_cache.clear();
    }

    buildProvinceOutlines();
}

auto HMDT::Project::MapProject::getProvinceProject() -> ProvinceProject& {
    return m_provinces_project;
}

auto HMDT::Project::MapProject::getProvinceProject() const
    -> const ProvinceProject&
{
    return m_provinces_project;
}

auto HMDT::Project::MapProject::getStateProject() -> StateProject& {
    return m_state_project;
}

auto HMDT::Project::MapProject::getStateProject() const -> const StateProject& {
    return m_state_project;
}

auto HMDT::Project::MapProject::getMapData() -> std::shared_ptr<MapData> {
    return m_map_data;
}

auto HMDT::Project::MapProject::getMapData() const
    -> const std::shared_ptr<MapData>
{
    return m_map_data;
}

const uint32_t* HMDT::Project::MapProject::getLabelMatrix() const {
    return m_map_data->getLabelMatrix().lock().get();
}

bool HMDT::Project::MapProject::isValidStateID(StateID state_id) const {
    return m_state_project.isValidStateID(state_id);
}

bool HMDT::Project::MapProject::isValidProvinceLabel(uint32_t label) const {
    return (label - 1) < m_provinces_project.getProvinces().size();
}

auto HMDT::Project::MapProject::getProvinceForLabel(uint32_t label) const
    -> const Province&
{
    return m_provinces_project.getProvinces().at(label - 1);
}

auto HMDT::Project::MapProject::getProvinceForLabel(uint32_t label)
    -> Province&
{
    return m_provinces_project.getProvinces().at(label - 1);
}

auto HMDT::Project::MapProject::getContinentList() const -> const ContinentSet&
{
    return m_continent_project.getContinentList();
}

auto HMDT::Project::MapProject::getContinents() -> ContinentSet& {
    return m_continent_project.getContinents();
}

auto HMDT::Project::MapProject::getTerrains() const
    -> const std::vector<Terrain>&
{
    return m_terrains;
}

/**
 * @brief Moves a province to another state.
 *
 * @param prov_id The ID of the province to move.
 * @param state_id The ID of the state to move the province to.
 */
void HMDT::Project::MapProject::moveProvinceToState(uint32_t prov_id,
                                                    StateID state_id)
{
    moveProvinceToState(getProvinceForLabel(prov_id), state_id);
}

/**
 * @brief Moves a province to another state.
 *
 * @param province The province to move.
 * @param state_id The ID of the state to move the province to.
 */
void HMDT::Project::MapProject::moveProvinceToState(Province& province,
                                                    StateID state_id)
{
    removeProvinceFromState(province);
    province.state = state_id;
    m_state_project.addProvinceToState(state_id, province.id);

    m_state_project.updateStateIDMatrix();
}

/**
 * @brief Removes a province from its state.
 *
 * @param province The province to remove.
 */
void HMDT::Project::MapProject::removeProvinceFromState(Province& province,
                                                        bool update_state_id_matrix)
{
    // Remove from its old state
    if(auto prov_state_id = province.state; isValidStateID(prov_state_id)) {
        m_state_project.removeProvinceFromState(prov_state_id, province.id);
    }
    province.state = -1;

    if(update_state_id_matrix) m_state_project.updateStateIDMatrix();
}

/**
 * @brief Gets province preview data for the given ID
 *
 * @param id
 *
 * @return The preview data, or nullptr if the ID does not exist
 */
auto HMDT::Project::MapProject::getPreviewData(ProvinceID id) -> ProvinceDataPtr
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
auto HMDT::Project::MapProject::getPreviewData(const Province* province_ptr)
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

auto HMDT::Project::MapProject::getProvinces() const -> const ProvinceList& {
    return m_provinces_project.getProvinces();
}

auto HMDT::Project::MapProject::getProvinces() -> ProvinceList& {
    return m_provinces_project.getProvinces();
}

auto HMDT::Project::MapProject::getStates() const
    -> const std::map<uint32_t, State>&
{
    return m_state_project.getStates();
}

/**
 * @brief Calculates whether each province is coastal or not
 * @details Will completely overwrite all manual coastal configuration done by
 *          the user.
 *
 * @param dry Whether or not this should actually modify the stored provinces
 */
void HMDT::Project::MapProject::calculateCoastalProvinces(bool dry) {
    WRITE_INFO("Calculating coastal provinces...");
    for(auto& province : getProvinces()) {
        // Only allow LAND provinces to be auto-marked as coastal
        //   I'm not actually sure if the game will allow LAKE and SEA to be
        //   coasts, but the cases where we would want that should be rare
        //   enough that I think it's fine to just require the user to configure
        //   that manually.
        if(province.type != ProvinceType::LAND) {
            WRITE_DEBUG("Skipping province ", province.id, " as it's not LAND.");
            continue;
        }

        // A province is coastal if it is adjacent to any SEA province.
        bool is_coastal = std::any_of(province.adjacent_provinces.begin(),
                                      province.adjacent_provinces.end(),
                                      [this](const auto& adj_prov_id) {
                                          return getProvinceForLabel(adj_prov_id).type == ProvinceType::SEA;
                                      });

        WRITE_DEBUG("Calculated that province '", province.id, "' is ",
                   (is_coastal ? "not " : ""), "coastal.");
        if(!dry) {
            province.coastal = is_coastal;
        } else {
            WRITE_DEBUG("Dry-Run enabled. Not modifying stored provinces.");
        }
    }
    WRITE_INFO("Done.");
}

auto HMDT::Project::MapProject::getStateForID(StateID state_id) const
    -> const State&
{
    // TODO
    return m_state_project.getStateForID(state_id)->get();
}

auto HMDT::Project::MapProject::getStateForID(StateID state_id) -> State& {
    // TODO
    return m_state_project.getStateForID(state_id)->get();
}

/**
 * @brief Will build the province preview for the given province. If more than
 *        MAX_CACHED_PROVINCE_PREVIEWS are already stored, then the least
 *        accessed preview will be kicked out of the cache before a new preview
 *        is constructed.
 *
 * @param province_ptr
 */
void HMDT::Project::MapProject::buildProvinceCache(const Province* province_ptr)
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
    auto label_matrix = m_map_data->getLabelMatrix().lock();
    auto iwidth = m_map_data->getWidth();

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

void HMDT::Project::MapProject::buildProvinceOutlines() {
    auto prov_outline_data = m_map_data->getProvinceOutlines().lock();
    auto graphics_data = m_map_data->getProvinces().lock();

    auto [width, height] = m_map_data->getDimensions();
    Dimensions dimensions{width, height};

    // Go over the map again and build extra data that depends on the previously
    //  re-built province data
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            auto lindex = xyToIndex(width, x, y);
            auto label = m_map_data->getLabelMatrix().lock()[lindex];
            auto gindex = xyToIndex(width * 4, x * 4, y);

            auto& province = getProvinceForLabel(label);

            // Error check
            if(label <= 0 || label > m_provinces_project.getProvinces().size()) {
                WRITE_WARN("Label matrix has label ", label,
                             " at position (", x, ',', y, "), which is out of "
                             "the range of valid labels [1,",
                             m_provinces_project.getProvinces().size(), "]");
                continue;
            }

            // Recalculate adjacencies for this pixel
            auto is_adjacent = ShapeFinder::calculateAdjacency(dimensions,
                                                               graphics_data.get(),
                                                               m_map_data->getLabelMatrix().lock().get(),
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

