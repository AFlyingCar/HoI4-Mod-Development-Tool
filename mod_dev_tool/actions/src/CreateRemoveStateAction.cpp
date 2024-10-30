
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
    m_type(Type::CREATE),
    m_on_value_changed_callback([](auto&&...){}),
    m_on_create_callback([](auto&&...){}),
    m_on_remove_callback([](auto&&...){})
{ }

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
    m_state_id(id),
    m_type(Type::REMOVE),
    m_on_value_changed_callback([](auto&&...){}),
    m_on_create_callback([](auto&&...){}),
    m_on_remove_callback([](auto&&...){})
{ }

bool HMDT::Action::CreateRemoveStateAction::doAction(const Callback& callback) {
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

auto HMDT::Action::CreateRemoveStateAction::onRemove(const OnValueChangedCallback& callback) noexcept
    -> CreateRemoveStateAction&
{
    m_on_remove_callback = callback;
    return *this;
}

bool HMDT::Action::CreateRemoveStateAction::create() {
    // TODO: WARNING!!!! This action will not create a state with the same ID as
    //   before when undoing! To fix this, StateProject needs a way to add a
    //   state along with an existing ID.
    m_state_id = m_history_project.getStateProject().addNewState(m_provinces);

    m_on_create_callback(m_state_id);
    m_on_value_changed_callback(m_state_id);

    return true;
}

bool HMDT::Action::CreateRemoveStateAction::remove() {
    auto maybe_state = m_history_project.getStateProject().getStateForID(m_state_id);
    RETURN_VALUE_IF_ERROR(maybe_state, false);

    m_provinces = maybe_state->get().provinces;
    m_history_project.getStateProject().removeState(m_state_id);

    m_on_remove_callback(m_state_id);
    m_on_value_changed_callback(m_state_id);

    return true;
}

