
#include "ActionManager.h"

auto MapNormalizer::Action::ActionManager::getInstance() -> ActionManager& {
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
bool MapNormalizer::Action::ActionManager::doAction(std::unique_ptr<IAction> action,
                                                    const IAction::Callback& callback)
{
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
bool MapNormalizer::Action::ActionManager::undoAction(const IAction::Callback& callback)
{
    // Stop early if there is nothing to undo
    if(!canUndo()) return false;

    if(auto& action = m_actions.top(); action->undoAction(callback)) {
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
bool MapNormalizer::Action::ActionManager::redoAction(const IAction::Callback& callback)
{
    // Stop early if there is nothing to redo
    if(!canRedo()) return false;

    if(auto& action = m_undone_actions.top(); action->doAction(callback)) {
        m_actions.push(std::move(action));
        m_undone_actions.pop();
        return true;
    }

    return false;
}

void MapNormalizer::Action::ActionManager::clearHistory() {
    m_actions = { };
    m_undone_actions = { };
}

bool MapNormalizer::Action::ActionManager::canUndo() const {
    return m_actions.size() > 0;
}

bool MapNormalizer::Action::ActionManager::canRedo() const {
    return m_undone_actions.size() > 0;
}

uint32_t MapNormalizer::Action::ActionManager::getHistorySize() const {
    return m_actions.size();
}

uint32_t MapNormalizer::Action::ActionManager::getUndoneHistorySize() const {
    return m_undone_actions.size();
}

MapNormalizer::Action::ActionManager::ActionManager(): m_actions() { }

