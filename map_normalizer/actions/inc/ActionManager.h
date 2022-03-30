#ifndef ACTIONMANAGER_H
# define ACTIONMANAGER_H

# include <stack>
# include <memory>
# include <functional>

# include "IAction.h"

namespace MapNormalizer::Action {
    /**
     * @brief The singleton manager of Actions
     *
     * @par This class is the main interface for how Actions are triggered. It
     *      maintains a state of each action performed, and is able to roll that
     *      stack backwards or forwards as necessary.
     */
    class ActionManager final {
        public:
            using ActionUpdateCallbackType = std::function<void(const IAction&)>;

            static ActionManager& getInstance();

            template<typename T>
            bool doAction(T* action,
                          const IAction::Callback& callback = IAction::_)
            {
                return doAction(std::unique_ptr<T>(action), callback);
            }

            bool doAction(std::unique_ptr<IAction>, const IAction::Callback& = IAction::_);
            bool undoAction(const IAction::Callback& = IAction::_);
            bool redoAction(const IAction::Callback& = IAction::_);

            void clearHistory();

            bool canUndo() const;
            bool canRedo() const;

            uint32_t getHistorySize() const;
            uint32_t getUndoneHistorySize() const;

            void setOnDoActionCallback(const ActionUpdateCallbackType&);
            void setOnUndoActionCallback(const ActionUpdateCallbackType&);
            void setOnRedoActionCallback(const ActionUpdateCallbackType&);

        private:
            ActionManager();

            //! Stack of actions from oldest to newest
            std::stack<std::unique_ptr<IAction>> m_actions;

            //! Stack of undone actions from newest to oldest
            std::stack<std::unique_ptr<IAction>> m_undone_actions;

            //! Called when an action is performed
            ActionUpdateCallbackType m_on_do_action;
            //! Called when an action is undone
            ActionUpdateCallbackType m_on_undo_action;
            //! Called when an action is redone
            ActionUpdateCallbackType m_on_redo_action;
    };
}

#endif

