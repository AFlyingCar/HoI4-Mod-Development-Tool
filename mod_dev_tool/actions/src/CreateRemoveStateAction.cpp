
#include "CreateRemoveStateAction.h"

#include "Logger.h"

/**
 * @brief Constructs a state creation action.
 *
 * @param history_project The history project
 * @param provinces The provinces to create a state with
 */
HMDT::Action::CreateRemoveStateAction::CreateRemoveStateAction(
        Project::IRootHistoryProject& history_project,
        const std::vector<ProvinceID>& provinces):
    m_history_project(history_project),
    m_provinces(provinces),
    m_old_province_to_states(), // Initialize in body
    m_type(Type::CREATE),
    m_is_valid(true),
    m_on_value_changed_callback([](auto&&...){return true;}),
    m_on_create_callback([](auto&&...){return true;}),
    m_on_remove_callback([](auto&&...){return true;})
{
    const auto& prov_project = history_project.getRootParent().getMapProject().getProvinceProject();

    for(auto&& id : provinces) {
        // If any province ID is invalid, then mark this action as invalid and
        //   stop immediately
        if(!(m_is_valid = prov_project.isValidProvinceID(id))) {
            return;
        }

        // Add in a mapping from the province to the current state so that we
        //   can undo it properly
        m_old_province_to_states[id] = prov_project.getProvinceForID(id).state;
    }
}

/**
 * @brief Constructs a state removal action.
 *
 * @param history_project The history project
 * @param id The id of the state to remove
 */
HMDT::Action::CreateRemoveStateAction::CreateRemoveStateAction(
        Project::IRootHistoryProject& history_project,
        const StateID& id):
    m_history_project(history_project),
    m_provinces(),
    m_old_province_to_states(), // Initialize in body
    m_state_id(id),
    m_type(Type::REMOVE),
    m_on_value_changed_callback([](auto&&...){return true;}),
    m_on_create_callback([](auto&&...){return true;}),
    m_on_remove_callback([](auto&&...){return true;})
{
    // Initializing a removal action means we need to also store the existing
    //   provinces
    if((m_is_valid = history_project.getStateProject().isValidStateID(id))) {
        // It's fine to just immediately de-reference the maybe, since we've already
        //   checked for validity
        const auto& state = history_project.getStateProject().getStateForID(id)->get();

        m_provinces = state.provinces;

        // Add every province ID to m_old_province_to_states
        for(auto&& prov_id : state.provinces) {
            m_old_province_to_states[prov_id] = id;
        }
    }
}

bool HMDT::Action::CreateRemoveStateAction::doAction(const Callback& callback) {
    if(!m_is_valid) return false;

    if(!callback(0)) return false;

    switch(m_type) {
        case Type::CREATE:
            if(!create()) return false;
            break;
        case Type::REMOVE:
            if(!remove()) return false;
            break;
    }

    if(!callback(1)) return false;

    return true;
}

bool HMDT::Action::CreateRemoveStateAction::undoAction(const Callback& callback)
{
    if(!callback(0)) return false;

    switch(m_type) {
        case Type::CREATE:
            if(!remove()) return false;
            break;
        case Type::REMOVE:
            if(!create()) return false;
            break;
    }

    if(!callback(1)) return false;

    return true;
}

auto HMDT::Action::CreateRemoveStateAction::onValueChanged(const OnValueChangedCallback& callback) noexcept
    -> CreateRemoveStateAction&
{
    m_on_value_changed_callback = callback;
    return *this;
}

auto HMDT::Action::CreateRemoveStateAction::onCreate(const OnValueChangedCallback& callback) noexcept
    -> CreateRemoveStateAction&
{
    m_on_create_callback = callback;
    return *this;
}

auto HMDT::Action::CreateRemoveStateAction::onRemove(const OnValueRemovedCallback& callback) noexcept
    -> CreateRemoveStateAction&
{
    m_on_remove_callback = callback;
    return *this;
}

bool HMDT::Action::CreateRemoveStateAction::create() {
    WRITE_DEBUG("Creating state with ", m_provinces.size(), " provinces.");

    // TODO: WARNING!!!! This action will not create a state with the same ID as
    //   before when undoing! To fix this, StateProject needs a way to add a
    //   state along with an existing ID.
    m_state_id = m_history_project.getStateProject().addNewState(m_provinces);

    if(!m_on_create_callback(m_state_id)) return false;
    if(!m_on_value_changed_callback(m_state_id)) return false;

    return true;
}

bool HMDT::Action::CreateRemoveStateAction::remove() {
    WRITE_DEBUG("Removing state ", m_state_id);

    auto maybe_state = m_history_project.getStateProject().getStateForID(m_state_id);
    RETURN_VALUE_IF_ERROR(maybe_state, false);

    if(!m_on_remove_callback(maybe_state->get())) return false;

    // Put all provinces back into their old states
    for(auto&& [prov_id, old_state_id] : m_old_province_to_states) {
        m_history_project.getRootParent().getMapProject().moveProvinceToState(prov_id, old_state_id);
    }

    auto result = m_history_project.getStateProject().removeState(m_state_id);
    RETURN_VALUE_IF_ERROR(result, false);

    if(!m_on_value_changed_callback(m_state_id)) return false;

    return true;
}

