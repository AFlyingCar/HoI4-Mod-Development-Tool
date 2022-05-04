
#include "ActionManager.h"

#include "Util.h"

auto HMDT::Action::ActionManager::getInstance() -> ActionManager& {
    static ActionManager instance;

    return instance;
}

/**
 * @brief Performs a single action.
 *
 * @param action The action to perform
 * @param callback A callback to be passed to the action
 *
 * @return True if the action was successful done, false otherwise.
 */
bool HMDT::Action::ActionManager::doAction(std::unique_ptr<IAction> action,
                                                    const IAction::Callback& callback)
{
    auto* action_ptr = action.get();
    RUN_AT_SCOPE_END([this, action_ptr]() { m_on_do_action(*action_ptr); });

    if(action->doAction(callback)) {
        // If the action cannot be undone, make sure we don't add it to the
        //  action history or clear the undo history
        if(action->canBeUndone()) {
            // Once an action is performed, all undone actions must be cleared,
            //  otherwise we end up in a situation where we have branching history,
            //  which just seems like a pain to manage
            m_undone_actions = { };
            m_actions.push(std::move(action));
        }
        return true;
    }

    return false;
}

/**
 * @brief Undoes the last action done.
 *
 * @param callback A callback to be passed to the action
 *
 * @return True if the action was successfully undone, false otherwise.
 */
bool HMDT::Action::ActionManager::undoAction(const IAction::Callback& callback)
{
    // Stop early if there is nothing to undo
    if(!canUndo()) return false;

    auto& action = m_actions.top();

    auto* action_ptr = action.get();
    RUN_AT_SCOPE_END([this, action_ptr]() { m_on_undo_action(*action_ptr); });

    if(action->undoAction(callback)) {
        m_undone_actions.push(std::move(action));
        m_actions.pop();
        return true;
    }

    return false;
}

/**
 * @brief Redoes the last action undone.
 * @details Note that this simply calls the 'do' method again on the action. If
 *          the action needs to know about whether it is being "done" vs
 *          "undone", then that state must be kept and managed by the action
 *          itself.
 *
 * @param callback A callback to be passed to the action
 *
 * @return True if the action was successfully redone, false otherwise.
 */
bool HMDT::Action::ActionManager::redoAction(const IAction::Callback& callback)
{
    // Stop early if there is nothing to redo
    if(!canRedo()) return false;

    auto& action = m_undone_actions.top();

    auto* action_ptr = action.get();
    RUN_AT_SCOPE_END([this, action_ptr]() { m_on_redo_action(*action_ptr); });

    if(action->doAction(callback)) {
        m_actions.push(std::move(action));
        m_undone_actions.pop();
        return true;
    }

    return false;
}

void HMDT::Action::ActionManager::clearHistory() {
    m_actions = { };
    m_undone_actions = { };
}

bool HMDT::Action::ActionManager::canUndo() const {
    return m_actions.size() > 0;
}

bool HMDT::Action::ActionManager::canRedo() const {
    return m_undone_actions.size() > 0;
}

uint32_t HMDT::Action::ActionManager::getHistorySize() const {
    return m_actions.size();
}

uint32_t HMDT::Action::ActionManager::getUndoneHistorySize() const {
    return m_undone_actions.size();
}

void HMDT::Action::ActionManager::setOnDoActionCallback(const ActionUpdateCallbackType& do_callback)
{
    m_on_do_action = do_callback;
}

void HMDT::Action::ActionManager::setOnUndoActionCallback(const ActionUpdateCallbackType& undo_callback)
{
    m_on_undo_action = undo_callback;
}

void HMDT::Action::ActionManager::setOnRedoActionCallback(const ActionUpdateCallbackType& redo_callback)
{
    m_on_redo_action = redo_callback;
}

HMDT::Action::ActionManager::ActionManager(): m_actions(),
                                                       m_undone_actions(),
                                                       m_on_do_action([](const auto&...) { }),
                                                       m_on_undo_action([](const auto&...) { }),
                                                       m_on_redo_action([](const auto&...) { })
{ }

