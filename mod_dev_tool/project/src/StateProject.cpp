
#include "StateProject.h"

#include <fstream>
#include <cstring>
#include <cerrno>

#include "Logger.h"

#include "Util.h"
#include "Options.h"
#include "Constants.h"
#include "StatusCodes.h"
#include "UniqueColorGenerator.h"

#include "HoI4Project.h"

HMDT::Project::StateProject::StateProject(MapProject& parent_project):
    m_parent_project(parent_project),
    m_available_state_ids(),
    m_states()
{
}

HMDT::Project::StateProject::~StateProject() { }

/**
 * @brief Writes all state data to root/$STATEDATA_FILENAME
 *
 * @param root The root where all state data should go
 * @param ec The error code
 *
 * @return True if state data was successfully saved, false otherwise
 */
auto HMDT::Project::StateProject::save(const std::filesystem::path& root) 
    -> MaybeVoid
{
    auto path = root / STATEDATA_FILENAME;

    if(std::ofstream out(path); out) {
        WRITE_DEBUG("Saving states to ", path);

        // FORMAT:
        //   ID;<State Name>;MANPOWER;<CATEGORY>;BUILDINGS_MAX_LEVEL_FACTOR;IMPASSABLE;PROVID1,PROVID2,...

        // TODO: We may end up supporting State history as well. If we do, then
        //   the best way to do so while still supporting this format is to
        //   have another file holding this info that's tied to the state
        //   (perhaps a 'hist/<STATEID>.hist' file)

        for(auto&& [_, state] : m_states) {
            WRITE_DEBUG("Writing state ID ", state.id);

            out << state.id << ';'
                << state.name << ';'
                << state.manpower << ';'
                << state.category << ';'
                << state.buildings_max_level_factor << ';'
                << (size_t)state.impassable << ';';

            for(ProvinceID p : state.provinces) {
                out << p << ',';
            }
            out << ';';

            out << static_cast<uint32_t>(state.color.r) << ';'
                << static_cast<uint32_t>(state.color.g) << ';'
                << static_cast<uint32_t>(state.color.b);

            out << std::endl;
        }
    } else {
        WRITE_ERROR("Failed to open file ", path);
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

/**
 * @brief Loads all state data from a file
 *
 * @param root The root where the state data file should be found
 * @param ec The error code
 *
 * @return True if data was loaded correctly, false otherwise
 */
auto HMDT::Project::StateProject::load(const std::filesystem::path& root)
    -> MaybeVoid
{
    auto path = root / STATEDATA_FILENAME;

    // If the file doesn't exist, then return false (we didn't actually load it
    //  after all), but don't set the error code as it is expected that the
    //  file may not exist
    if(std::error_code ec; !std::filesystem::exists(path, ec)) {
        RETURN_ERROR_IF(ec.value() != 0, ec);

        WRITE_WARN("No data to load! No states currently exist!");
        return std::make_error_code(std::errc::no_such_file_or_directory);
    }

    if(std::ifstream in(path); in) {
        // Make sure we clear out the states map first just in case there is
        //   some data in here (there shouldn't be)
        m_states.clear();

        // FORMAT:
        //   ID;<State Name>;MANPOWER;<CATEGORY>;BUILDINGS_MAX_LEVEL_FACTOR;IMPASSABLE;PROVID1,PROVID2,...
        std::string line;
        for(size_t line_num = 0; std::getline(in, line); ++line_num) {
            if(line.empty()) continue;

            std::istringstream iss(line);

            State state;
            state.color = Color{0,0,0}; // Initialize this to nothing

            std::string prov_id_data;
            if(!parseValuesSkipMissing<';'>(iss, &state.id,
                                                 &state.name,
                                                 &state.manpower,
                                                 &state.category,
                                                 &state.buildings_max_level_factor,
                                                 &state.impassable,
                                                 &prov_id_data, true,
                                                 &state.color.r, true,
                                                 &state.color.g, true,
                                                 &state.color.b, true))
            {
                WRITE_ERROR("Failed to parse line #", line_num, ": '", line, "'");
                RETURN_ERROR(std::make_error_code(std::errc::bad_message));
            }

            WRITE_DEBUG("Reading state data {"
                        "id=", state.id, ", "
                        "name=", state.name, ", "
                        "manpower=", state.manpower, ", "
                        "category=", state.category, ", "
                        "buildings_max_level_factor=", state.buildings_max_level_factor, ", "
                        "impassable=", state.impassable, ", "
                        "prov_id_data=<DATA>, "
                        "color={",
                        "r=", state.color.r, ", "
                        "g=", state.color.g, ", "
                        "b=", state.color.b, "}"
                        "}"
            );

            // If we did not load a state color, then the color should be 0,0,0
            // In that case, we want to generate a new unique color value
            if(state.color == Color{0,0,0}) {
                WRITE_WARN("Saved state data did not have a color value, generating a new one...");
                state.color = generateUniqueColor(ProvinceType::UNKNOWN);
            } else {
                generateUniqueColor(ProvinceType::UNKNOWN); // "generate" a color to advance the number of colors by 1
            }

            // We need to parse the provinces seperately
            state.provinces = splitAndTransform<ProvinceID>(prov_id_data, ',',
                    [](const std::string& v) {
                        return std::atoi(v.c_str());
                    });

            if(m_states.count(state.id) != 0) {
                WRITE_ERROR("Found multiple states with the same ID of ", state.id, "! We will skip the second one '", state.name, "' and keep '", m_states.at(state.id).name, '\'');
            } else {
                WRITE_DEBUG("Successfully loaded state ID ", state.id,
                            " named ", state.name);
                m_states[state.id] = state;
            }
        }

        // Now that we've loaded every single state, we need to track which IDs
        //  have not been used yet
        for(StateID id = 1; id < m_states.size(); ++id) {
            if(m_states.count(id) == 0) {
                m_available_state_ids.push(id);
            }
        }

        updateStateIDMatrix();
    } else {
        WRITE_ERROR("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
    }

    return STATUS_SUCCESS;
}

auto HMDT::Project::StateProject::export_(const std::filesystem::path& root) const noexcept
    -> MaybeVoid
{
    // First create the export path if it doesn't exist
    if(std::error_code fs_ec; !std::filesystem::exists(root, fs_ec)) {
        RETURN_ERROR_IF(fs_ec.value() != 0 &&
                        fs_ec != std::errc::no_such_file_or_directory,
                        fs_ec);

        // TODO: This is temporary. It should just be create_directory
        //   But since StateProject is not part of HistoryProject yet, we have
        //    to create our entire path from root
        auto result = std::filesystem::create_directories(root, fs_ec);

        RETURN_ERROR_IF(!result, fs_ec);
    }

    for(auto&& [id, state] : m_states) {
        auto filename = std::to_string(id) + "-" + state.name + ".txt";
        auto state_path = root / filename;

        if(std::ofstream out(state_path); out) {
            std::stringstream provinces_ss;
            for(auto&& id : state.provinces) {
                provinces_ss << id << ' ';
            }

            out << "state={" << std::endl;
            // General state information
            out << "\tid=" << id << std::endl;
            out << "\tname=\"" << state.name << '"' << std::endl; // TODO: HoI4 uses STATE_{ID} here, is that for localization?
            out << "\tmanpower=" << state.manpower << std::endl;
            out << "\tstate_category = " << state.category << std::endl;

            // Leave this out of the export if it's left as the default 1.0
            if(state.buildings_max_level_factor != 1.0) {
                // TODO: wiki recommends avoiding this. Should we not support it at all?
                out << "\tbuildings_max_level_factor=" << state.buildings_max_level_factor << std::endl;
            }

            // TODO: Resources

            if(state.impassable) {
                out << "\timpassable = yes" << std::endl;
            }

            // History here
            out << "\thistory={" << std::endl;

            // NOTE (from wiki):
            //   Only one province can be defined within one victory_points.
            //   In order to have multiple provinces with victory points in one
            //   state, several instances of victory_points = { ... } need to be
            //   put in.
            // TODO: This should be a for-loop, generating a 'victory_points={}'
            //   block for each victory point
            // out << "\t\tvictory_points={" << std::endl;
            // TODO Format is "PROVID AMOUNT"
            // out << "\t\t}" << std::endl;

            // TODO: Owner
            //   Game will load without owners, but doing stuff to this state
            //   (like transferring it) will cause a crash
            // For now, we are using a country that does not exist at the start
            //   of the game and has no focus tree for testing.
            out << "\t\towner = CHA" << std::endl;

            out << "\t\tbuildings={" << std::endl;
            // TODO
            //  NOTE: Each of these can be left blank if their count is 0
            //  NOTE: When designing how these buildings are outputted, we
            //    should keep in mind that custom buildings can be added as well
            //
            //  infrastructure = ...
            //  arms_factory = ...
            //  industrial_complex = ...
            //  dockyard = ...
            //  airbase = ... // TODO: air_base? wiki disagrees with what's in the files
            //  anti_air_building = ...
            //  synthetic_refinery = ...
            //  fuel_silo = ...
            //  radar_station = ...
            //  rocket_site = ...
            //  nuclear_reactor = ...
            //  for each province: // Skip if province has no buildings
            //    id = {
            //      naval_base = ...
            //      bunker = ...
            //      coastal_bunker = ...
            //      supply_node = ...
            //      rail_way = ...
            //    }
            out << "\t\t}" << std::endl;

            // TODO
            //  This is optional, for if someone other than the owner should
            //  start out controlling it
            // out << "\t\tcontroller = " << std::endl;

            // TODO
            // Optional, for if claimed by another country
            // out << "\t\tadd_core_of = " << std::endl;

            // TODO: This serves as an effect block. Do we want to allow
            //   defining other effects on a state?

            out << "\t}" << std::endl;
            // More general state information
            out << "\tprovinces={" << std::endl;
            out << "\t\t" << provinces_ss.str() << std::endl;
            out << "\t}" << std::endl;
            // TODO
            //   This is optional, it is for defining the base supply of the
            //    state
            // out << "\tlocal_supplies=" << ... << std::endl;
            out << "}";
        } else {
            WRITE_ERROR("Failed to open file ", state_path);
            RETURN_ERROR(std::make_error_code(static_cast<std::errc>(errno)));
        }
    }

    return STATUS_SUCCESS;
}

void HMDT::Project::StateProject::import(const ShapeFinder& sf, std::shared_ptr<MapData> map_data)
{
}

auto HMDT::Project::StateProject::getMapData() -> std::shared_ptr<MapData> {
    return m_parent_project.getMapData();
}

auto HMDT::Project::StateProject::getMapData() const
    -> const std::shared_ptr<MapData>
{
    return m_parent_project.getMapData();
}

bool HMDT::Project::StateProject::validateData() {
    // We have nothing to really validate here
    return true;
}

auto HMDT::Project::StateProject::validateProvinceStateID(StateID province_state_id, ProvinceID province_id)
    -> MaybeVoid
{
    if(province_state_id != -1) {
        if(!isValidStateID(province_state_id)) {
            WRITE_WARN("Province has state ID of ", province_state_id, " which is invalid!");
            return STATUS_PROVINCE_INVALID_STATE_ID;
        }

        MaybeRef<State> state = getStateForID(province_state_id);
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


HMDT::Project::IRootProject& HMDT::Project::StateProject::getRootParent() {
    return m_parent_project.getRootParent();
}

HMDT::Project::IMapProject& HMDT::Project::StateProject::getRootMapParent() {
    return m_parent_project.getRootMapParent();
}

auto HMDT::Project::StateProject::getStateMap() -> StateMap& {
    return m_states;
}

auto HMDT::Project::StateProject::getStates() const -> const StateMap& {
    return m_states;
}

void HMDT::Project::StateProject::updateStateIDMatrix() {
    auto state_id_matrix = getMapData()->getStateIDMatrix().lock();

    auto label_matrix = getMapData()->getLabelMatrix().lock();

    auto* label_matrix_start = label_matrix.get();

    parallelTransform(label_matrix_start, label_matrix_start + getMapData()->getMatrixSize(),
                      state_id_matrix.get(),
                      [this](uint32_t prov_id) -> uint32_t {
                          if(m_parent_project.isValidProvinceLabel(prov_id)) {
                              return m_parent_project.getProvinceForLabel(prov_id).state;
                          } else {
                              WRITE_WARN("Invalid province ID ", prov_id,
                                         " detected when building state id matrix. Treating as though there's no state here.");
                              return 0;
                          }
                      });

    if(prog_opts.debug) {
        auto path = getRootParent().getDebugRoot();
        auto fname = path / "stateidmtx.txt";

        if(!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
        }

        WRITE_DEBUG("Writing state id matrix to ", fname);

        if(std::ofstream out(fname); out) {
            auto [width, height] = getMapData()->getDimensions();
            uint32_t i = 0;
            for(auto y = 0; y < height; ++y) {
                for(auto x = 0; x < width; ++x) {
                    out << state_id_matrix[i] << ' ';
                    ++i;
                }
                out << '\n';
            }
        }
    }

    WRITE_DEBUG("Done updating State ID matrix.");
}

/**
 * @brief Creates a new state composed of all provinces in province_ids.
 * @details The provinces detailed in province_ids will get removed from their
 *          original states. Note however that the original states will still
 *          exist.
 *
 * @param province_ids The list of provinces to add to the new state.
 *
 * @return The ID of the new state
 */
auto HMDT::Project::StateProject::addNewState(const std::vector<uint32_t>& province_ids)
    -> StateID
{
    RefVector<Province> provinces;
    std::transform(province_ids.begin(), province_ids.end(),
                   std::back_inserter(provinces),
                   [this](uint32_t prov_id) -> std::reference_wrapper<Province> {
                       return std::ref(m_parent_project.getProvinceForLabel(prov_id));
                   });

    // Increment by 1 because we do not want 0 to be a state ID
    StateID id = m_states.size() + 1;
    if(m_available_state_ids.size() > 0) {
        id = m_available_state_ids.front();
        m_available_state_ids.pop();
    }

    WRITE_DEBUG("Creating new state with ID ", id);

    // Make sure that the provinces are decoupled from their original state
    for(auto&& prov : provinces) {
        m_parent_project.removeProvinceFromState(prov, false);
        prov.get().state = id;
    }

    // Note that we default the name to 'STATE#'
    using namespace std::string_literals;
    m_states[id] = State {
        id,
        "STATE"s + std::to_string(id), /* name */
        0, /* manpower */
        "", /* category */
        DEFAULT_BUILDINGS_MAX_LEVEL_FACTOR, /* buildings_max_level_factor */
        false, /* impassable */
        province_ids,
        generateUniqueColor(ProvinceType::UNKNOWN)
    };

    updateStateIDMatrix();

    return id;
}

/**
 * @brief Deletes the state at id
 *
 * @param id The ID of the state to delete
 */
void HMDT::Project::StateProject::removeState(StateID id) {
    m_available_state_ids.push(id);
    m_states.erase(id);

    updateStateIDMatrix();
}

auto HMDT::Project::StateProject::getStateForIterator(StateMap::const_iterator cit)
    -> State&
{
    // Lookup the state again using the key
    return getStateMap().at(cit->first);
}

auto HMDT::Project::StateProject::getStateForIterator(StateMap::const_iterator cit) const
    -> const State&
{
    return cit->second;
}

auto HMDT::Project::StateProject::addProvinceToState(StateID state_id,
                                                     ProvinceID province_id)
    -> MaybeVoid
{
    return getStateForID(state_id).andThen([&province_id](auto state_ref)
    {
        state_ref.get().provinces.push_back(province_id);
    });
}

auto HMDT::Project::StateProject::removeProvinceFromState(StateID state_id,
                                                          ProvinceID province_id)
    -> MaybeVoid
{
    return getStateForID(state_id).andThen([&province_id](auto state_ref)
    {
        auto& state = state_ref.get();
        for(auto it = state.provinces.begin(); it != state.provinces.end(); ++it)
        {
            if(*it == province_id) {
                state.provinces.erase(it);
                break;
            }
        }
    });
}

