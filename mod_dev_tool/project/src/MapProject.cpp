
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

#include "ProjectNode.h"

HMDT::Project::MapProject::MapProject(IProject& parent_project):
    m_provinces_project(*this),
    m_continent_project(*this),
    m_heightmap_project(*this),
    m_rivers_project(*this),
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

    MaybeVoid result = STATUS_SUCCESS;

    result = m_provinces_project.save(path);
    if(result == STATUS_NO_DATA_LOADED) {
        result = STATUS_SUCCESS;
    }
    RETURN_IF_ERROR(result);

    result = m_continent_project.save(path);
    if(result == STATUS_NO_DATA_LOADED) {
        result = STATUS_SUCCESS;
    }
    RETURN_IF_ERROR(result);

    result = m_heightmap_project.save(path);
    if(result == STATUS_NO_DATA_LOADED) {
        result = STATUS_SUCCESS;
    }
    RETURN_IF_ERROR(result);

    result = m_rivers_project.save(path);
    if(result == STATUS_NO_DATA_LOADED) {
        result = STATUS_SUCCESS;
    }
    RETURN_IF_ERROR(result);

    return result;
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

    auto inputs_root = getRootParent().getInputsRoot();
    auto input_provincemap_path = inputs_root / INPUT_PROVINCEMAP_FILENAME;
    if(!std::filesystem::exists(input_provincemap_path)) {
        WRITE_WARN("Source import image does not exist, unable to finish loading data.");
        RETURN_ERROR(std::make_error_code(std::errc::no_such_file_or_directory));
    } else {
        readBMP(input_provincemap_path, input_image.get());

        if(input_image == nullptr) {
            // TODO: We should instead have readBMP() return an appropriate
            //       error code rather than assuming one.
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

    auto input_data = m_map_data->getInput().lock();

    // Copy the input image's data into the input_data
    std::copy(input_data.get(), input_data.get() + m_map_data->getInputSize(),
              input_image->data);

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

    if(auto result = m_heightmap_project.load(path);
            result.error() != std::errc::no_such_file_or_directory)
    {
        RETURN_IF_ERROR(result);
    }

    if(auto result = m_rivers_project.load(path);
            result.error() != std::errc::no_such_file_or_directory)
    {
        RETURN_IF_ERROR(result);
    }

    return STATUS_SUCCESS;
}

auto HMDT::Project::MapProject::export_(const std::filesystem::path& root) const noexcept
    -> MaybeVoid
{
    WRITE_DEBUG("Exporting to ", root);

    // First create the export path if it doesn't exist
    if(std::error_code fs_ec; !std::filesystem::exists(root, fs_ec)) {
        RETURN_ERROR_IF(fs_ec.value() != 0 &&
                        fs_ec != std::errc::no_such_file_or_directory,
                        fs_ec);

        auto result = std::filesystem::create_directory(root, fs_ec);

        RETURN_ERROR_IF(!result, fs_ec);
    }

    MaybeVoid result;

    result = m_provinces_project.export_(root);
    RETURN_IF_ERROR(result);

    result = m_continent_project.export_(root);
    RETURN_IF_ERROR(result);

    result = m_heightmap_project.export_(root);
    RETURN_IF_ERROR(result);

    result = m_rivers_project.export_(root);
    RETURN_IF_ERROR(result);

    ////////////////////////////////////////////////////////////////////////////
    // TODO: These are files that will still be required by HoI4, but which we
    //  have no editing capabilities for and which can be left blank

    // Adjacencies
    {
        // adjacencies.csv
        if(std::ofstream adjacencies(root / "adjacencies.csv"); adjacencies) {
            adjacencies << "From;To;Type;Through;start_x;start_y;stop_x;stop_y;adjacency_rule_name;Comment";
            // TODO
        } else {
            WRITE_ERROR("Failed to open file ", root / "adjacencies.csv");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        // adjacency_rules.txt
        if(std::ofstream adjacency_rules(root / "adjacency_rules.txt"); adjacency_rules)
        {
            // TODO
        } else {
            WRITE_ERROR("Failed to open file ", root / "adjacency_rules.txt");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }
    }

    // Buildings
    {
        // NOTE: These files can be modified with the Nudge tool, but they need to
        //  at least exist first and have at least one entry
        //  (otherwise the game could crash)
        // TODO: Do we want to try and replicate the Nudge tool?

        // buildings.txt
        if(std::ofstream buildings(root / "buildings.txt"); buildings) {
            // State ID (integer); building ID (string); X position; Y position; Z position; Rotation; Adjacent sea province (integer)
            buildings << "1;arms_factory;0;0;0;0;0";
        } else {
            WRITE_ERROR("Failed to open file ", root / "buildings.txt");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        // airports.txt
        //  Which province the airports appear in for each state
        if(std::ofstream airports(root / "airports.txt"); airports) {
            // State ID (integer)={province id }
            airports << "1={1 }";
        } else {
            WRITE_ERROR("Failed to open file ", root / "airports.txt");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }

        // rocketsites.txt
        //  Which province the rocketsites appear in for each state
        if(std::ofstream rocketsites(root / "rocketsites.txt"); rocketsites) {
            // State ID (integer)={province id }
            rocketsites << "1={1 }";
        } else {
            WRITE_ERROR("Failed to open file ", root / "rocketsites.txt");
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }
    }

    // Cities
    {
        // TODO: We need to come up with a custom cities.bmp to match the
        //  dimensions of the map, but how is this file actually meant to be
        //  used? The wiki doesn't really say, and other mods i can see tend to
        //  leave it just blank.

        // cities.bmp
        {
            // Fill the map with 0x081F82
            {
                auto cities_data = getMapData()->getCities().lock();
                std::generate(cities_data.get(),
                              cities_data.get() + getMapData()->getCitiesSize(),
                              [i = 0]() mutable {
                                  constexpr uint8_t cdata[] = { 0x82, 0x1F, 0x08 };
                                  auto v = cdata[i % 3];
                                  i = (i+1) % 3;
                                  return v;
                              });
            }

            // TODO: writeBMP does not actually return any errors out to us, so we
            //  need to be careful here in case it does fail
            writeBMP(root / CITIESBMP_FILENAME,
                     getMapData()->getCities().lock().get(),
                     getMapData()->getWidth(), getMapData()->getHeight());
        }

        // cities.txt
        // TODO: My understanding is that this defines how cities.bmp should be
        //   interpreted. Should we output a custom one?
    }

    return STATUS_SUCCESS;
}

auto HMDT::Project::MapProject::validateProvinceStateID(StateID province_state_id,
                                                        ProvinceID province_id)
    -> MaybeVoid
{
    if(province_state_id != -1) {
        const auto& state_project = getRootParent().getHistoryProject().getStateProject();

        if(!state_project.isValidStateID(province_state_id)) {
            WRITE_WARN("Province ", province_id, " has state ID of ", province_state_id, " which is invalid!");
            return STATUS_PROVINCE_INVALID_STATE_ID;
        }

        MaybeRef<const State> state = state_project.getStateForID(province_state_id);
        return state.andThen([&](auto state_ref) {
            auto& state = state_ref.get();
            if(std::find(state.provinces.begin(), state.provinces.end(), province_id) == state.provinces.end())
            {
                WRITE_WARN("State ", state.id, " does not contain province #", province_id, "!");

                return STATUS_PROVINCE_NOT_IN_STATE;
            }

            return STATUS_SUCCESS;
        });
    }

    return STATUS_SUCCESS;
}

bool HMDT::Project::MapProject::validateData() {
    WRITE_DEBUG("Validating all project data.");

    bool success = true;

    success = success && m_provinces_project.validateData();

    for(auto&& [_, local_province] : m_provinces_project.getProvinces()) {
        auto& province = local_province;

        auto result = validateProvinceStateID(province.state, province.id);

        if(IS_FAILURE(result)) {
            if(prog_opts.fix_warnings_on_load) {
                WRITE_INFO("Attempting to fix warnings...");
                if(result.error() == STATUS_PROVINCE_INVALID_STATE_ID) {
                    WRITE_INFO("Setting province state ID to -1.");
                    province.state = -1;
                } else if(result.error() == STATUS_PROVINCE_NOT_IN_STATE) {
                    getRootParent().getHistoryProject().getStateProject().getStateForID(province.state).andThen([&](auto prov_state_ref)
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
                    auto it = std::find_if(getRootParent().getHistoryProject().getStateProject().getStates().begin(),
                                           getRootParent().getHistoryProject().getStateProject().getStates().end(),
                                           [&province, &province_it](auto& id_state_pair)
                                           {
                                               return (province_it = std::find(id_state_pair.second.provinces.begin(),
                                                                               id_state_pair.second.provinces.end(),
                                                                               province.id)) != id_state_pair.second.provinces.end();
                                           });

                    if(it == getRootParent().getHistoryProject().getStateProject().getStates().end()) {
                        WRITE_INFO("No states found containing this province.");
                    } else {
                        WRITE_INFO("Found state ", it->second.id, " that contains province ", province.id, ". Removing the province from the state.");
                        State& state = getRootParent().getHistoryProject().getStateProject().getStateForIterator(it);
                        state.provinces.erase(province_it);
                    }
                }
            } else {
                success = false;
            }

            continue;
        }
    }

    return success;
}

HMDT::Project::IRootProject& HMDT::Project::MapProject::getRootParent() {
    return m_parent_project.getRootParent();
}

const HMDT::Project::IRootProject& HMDT::Project::MapProject::getRootParent() const
{
    return m_parent_project.getRootParent();
}

HMDT::Project::IRootMapProject& HMDT::Project::MapProject::getRootMapParent() {
    return *this;
}

const HMDT::Project::IRootMapProject& HMDT::Project::MapProject::getRootMapParent() const {
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
    }
}

auto HMDT::Project::MapProject::getProvinceProject() noexcept
    -> ProvinceProject&
{
    return m_provinces_project;
}

auto HMDT::Project::MapProject::getProvinceProject() const noexcept
    -> const ProvinceProject&
{
    return m_provinces_project;
}

auto HMDT::Project::MapProject::getContinentProject() noexcept
    -> ContinentProject&
{
    return m_continent_project;
}

auto HMDT::Project::MapProject::getContinentProject() const noexcept
    -> const ContinentProject&
{
    return m_continent_project;
}

auto HMDT::Project::MapProject::getRiversProject() noexcept
    -> RiversProject&
{
    return m_rivers_project;
}

auto HMDT::Project::MapProject::getRiversProject() const noexcept
    -> const RiversProject&
{
    return m_rivers_project;
}

auto HMDT::Project::MapProject::getMapData() -> std::shared_ptr<MapData> {
    return m_map_data;
}

auto HMDT::Project::MapProject::getMapData() const
    -> const std::shared_ptr<MapData>
{
    return m_map_data;
}

auto HMDT::Project::MapProject::getHeightMapProject() noexcept
    -> HeightMapProject&
{
    return m_heightmap_project;
}

auto HMDT::Project::MapProject::getHeightMapProject() const noexcept
    -> const HeightMapProject&
{
    return m_heightmap_project;
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
void HMDT::Project::MapProject::moveProvinceToState(ProvinceID prov_id,
                                                    StateID state_id)
{
    moveProvinceToState(getProvinceProject().getProvinceForID(prov_id), state_id);
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
    getRootParent().getHistoryProject().getStateProject().addProvinceToState(state_id, province.id);

    getRootParent().getHistoryProject().getStateProject().updateStateIDMatrix();
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
    if(auto prov_state_id = province.state;
            getRootParent().getHistoryProject().getStateProject().isValidStateID(prov_state_id))
    {
        getRootParent().getHistoryProject().getStateProject().removeProvinceFromState(prov_state_id, province.id);
    }
    province.state = -1;

    if(update_state_id_matrix) getRootParent().getHistoryProject().getStateProject().updateStateIDMatrix();
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
    for(auto&& [_, province] : getProvinceProject().getProvinces()) {
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
                                          return m_provinces_project.getProvinceForID(adj_prov_id).type == ProvinceType::SEA;
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

auto HMDT::Project::MapProject::visit(const std::function<MaybeVoid(std::shared_ptr<Hierarchy::INode>)>& visitor) const noexcept
    -> Maybe<std::shared_ptr<Hierarchy::INode>>
{
    auto map_project_node = std::make_shared<Hierarchy::ProjectNode>("Map");

    auto result = visitor(map_project_node);
    RETURN_IF_ERROR(result);

    result = getProvinceProject().visit(visitor)
        .andThen([&map_project_node](auto province_project_node) -> MaybeVoid {
            auto result = map_project_node->addChild(province_project_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    result = getContinentProject().visit(visitor)
        .andThen([&map_project_node](auto continent_project_node) -> MaybeVoid {
            auto result = map_project_node->addChild(continent_project_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    result = getHeightMapProject().visit(visitor)
        .andThen([&map_project_node](auto heightmap_project_node) -> MaybeVoid {
            auto result = map_project_node->addChild(heightmap_project_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    result = getRiversProject().visit(visitor)
        .andThen([&map_project_node](auto rivers_project_node) -> MaybeVoid {
            auto result = map_project_node->addChild(rivers_project_node);
            RETURN_IF_ERROR(result);

            return STATUS_SUCCESS;
        });
    RETURN_IF_ERROR(result);

    return map_project_node;
}

