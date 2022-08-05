
#include "SelectionManager.h"

#include <algorithm>

#include "Driver.h"

auto HMDT::GUI::SelectionManager::getInstance() -> SelectionManager& {
    static SelectionManager instance;

    return instance;
}

void HMDT::GUI::SelectionManager::selectProvince(uint32_t label) {
    // Do not select if the label isn't valid
    if(auto opt_mproj = getCurrentMapProject(); opt_mproj && opt_mproj->get().isValidProvinceLabel(label))
    {
        m_on_province_selected_callback(label, Action::SET);
        m_selected_provinces = {label};
    }
}

void HMDT::GUI::SelectionManager::addProvinceSelection(uint32_t label) {
    // Do not select if the label isn't valid
    if(auto opt_mproj = getCurrentMapProject(); opt_mproj && opt_mproj->get().isValidProvinceLabel(label))
    {
        m_on_province_selected_callback(label, Action::ADD);
        m_selected_provinces.insert(label);
    }
}

void HMDT::GUI::SelectionManager::removeProvinceSelection(uint32_t label) {
    m_on_province_selected_callback(label, Action::REMOVE);
    m_selected_provinces.erase(label);
}

void HMDT::GUI::SelectionManager::clearProvinceSelection() {
    m_on_province_selected_callback(INVALID_PROVINCE_ID, Action::CLEAR);
    m_selected_provinces.clear();
}

void HMDT::GUI::SelectionManager::selectState(StateID state_id) {
    // Do not select if the state_id isn't valid
    if(auto opt_mproj = getCurrentMapProject(); opt_mproj && opt_mproj->get().isValidStateID(state_id))
    {
        m_on_state_selected_callback(state_id, Action::SET);
        m_selected_states = {state_id};
    }
}

void HMDT::GUI::SelectionManager::addStateSelection(StateID state_id) {
    // Do not select if the state_id isn't valid
    if(auto opt_mproj = getCurrentMapProject(); opt_mproj && opt_mproj->get().isValidStateID(state_id))
    {
        m_on_state_selected_callback(state_id, Action::ADD);
        m_selected_states.insert(state_id);
    }
}

void HMDT::GUI::SelectionManager::removeStateSelection(StateID state_id) {
    m_on_state_selected_callback(state_id, Action::REMOVE);
    m_selected_states.erase(state_id);
}

void HMDT::GUI::SelectionManager::clearStateSelection() {
    m_on_state_selected_callback(INVALID_STATE_ID, Action::CLEAR);
    m_selected_states.clear();
}

void HMDT::GUI::SelectionManager::setOnSelectProvinceCallback(const std::function<void(uint32_t, Action)>& on_province_selected_callback)
{
    m_on_province_selected_callback = on_province_selected_callback;
}

void HMDT::GUI::SelectionManager::setOnSelectStateCallback(const std::function<void(StateID, Action)>& on_state_selected_callback)
{
    m_on_state_selected_callback = on_state_selected_callback;
}

size_t HMDT::GUI::SelectionManager::getSelectedProvinceCount() const {
    return m_selected_provinces.size();
}

size_t HMDT::GUI::SelectionManager::getSelectedStateCount() const {
    return m_selected_states.size();
}

/**
 * @brief Will return the currently selected provinces.
 *
 * @return The currently selected provinces.
 */
auto HMDT::GUI::SelectionManager::getSelectedProvinces() const
    -> RefVector<const Province>
{
    RefVector<const Province> provinces;

    if(auto opt_mproj = getCurrentMapProject(); opt_mproj)
    {
        auto& mproj = opt_mproj->get();
        std::transform(m_selected_provinces.begin(), m_selected_provinces.end(),
                       std::back_inserter(provinces),
                       [&mproj](uint32_t prov_id) {
                           return std::ref(mproj.getProvinceForLabel(prov_id));
                       });
    }

    return provinces;
}

/**
 * @brief Will return the currently selected provinces.
 *
 * @return The currently selected provinces.
 */
auto HMDT::GUI::SelectionManager::getSelectedProvinces() -> RefVector<Province>
{
    RefVector<Province> provinces;
    if(auto opt_mproj = getCurrentMapProject(); opt_mproj)
    {
        auto& mproj = opt_mproj->get();
        std::transform(m_selected_provinces.begin(), m_selected_provinces.end(),
                       std::back_inserter(provinces),
                       [&mproj](uint32_t prov_id) {
                           return std::ref(mproj.getProvinceForLabel(prov_id));
                       });
    }
    return provinces;
}

auto HMDT::GUI::SelectionManager::getSelectedProvinceLabels() const
    -> const std::set<uint32_t>&
{
    return m_selected_provinces;
}

/**
 * @brief Will return the currently selected states.
 *
 * @return The currently selected states.
 */
auto HMDT::GUI::SelectionManager::getSelectedStates() const
    -> RefVector<const State>
{
    RefVector<const State> states;
    if(auto opt_mproj = getCurrentMapProject(); opt_mproj)
    {
        auto& mproj = opt_mproj->get();
        std::transform(m_selected_states.begin(), m_selected_states.end(),
                       std::back_inserter(states),
                       [mproj](StateID state_id) -> const State& {
                           return mproj.getStateForID(state_id)->get();
                       });
    }
    return states;
}

/**
 * @brief Will return the currently selected states.
 *
 * @return The currently selected states.
 */
auto HMDT::GUI::SelectionManager::getSelectedStates() -> RefVector<State> {
    RefVector<State> states;
    if(auto opt_mproj = getCurrentMapProject(); opt_mproj)
    {
        auto& mproj = opt_mproj->get();
        std::transform(m_selected_states.begin(), m_selected_states.end(),
                       std::back_inserter(states),
                       [&mproj](StateID state_id) -> State& {
                           return mproj.getStateForID(state_id)->get();
                       });
    }
    return states;
}

auto HMDT::GUI::SelectionManager::getSelectedStateIDs() const
    -> const std::set<uint32_t>&
{
    return m_selected_states;
}

/**
 * @brief Clears out all selection information
 */
void HMDT::GUI::SelectionManager::onProjectUnloaded() {
    m_selected_provinces.clear();
    m_selected_states.clear();
}

HMDT::GUI::SelectionManager::SelectionManager():
    m_selected_provinces(),
    m_selected_states(),
    m_on_province_selected_callback([](auto...) { }),
    m_on_state_selected_callback([](auto...) { })
{ }

auto HMDT::GUI::SelectionManager::getCurrentMapProject() const
    -> OptionalReference<Project::MapProject>
{
    if(auto opt_project = Driver::getInstance().getProject(); opt_project) {
        return std::ref(opt_project->get().getMapProject());
    } else {
        return std::nullopt;
    }
}

