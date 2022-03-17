#ifndef IACTION_H
# define IACTION_H

# include <functional>

namespace MapNormalizer::Action {
    /**
     * @brief Interface class for defining the basics of what makes up an Action
     *
     * @par
     *      Every action is composed of two parts: The 'DO' and the 'UNDO' parts.
     *      Generally, 'DO' should perform some operation to change state, and
     *      should retain enough information about that state to know how to
     *      later reverse it in the 'UNDO' function.
     *
     * @par
     *      Both 'DO' and 'UNDO' take a callback function, which itself takes an
     *      ActionState and returns a boolean. This ActionState does not
     *      necessarily need to be used, but it is meant to represent what the
     *      current state of the DO/UNDO operation is at, in case the action
     *      decides to invoke the callback multiple times (say, for
     *      longer-running operations that may be made up of many different
     *      parts). Additionally, the boolean is meant to return some
     *      information back to the action, should it choose to act on it. This
     *      could be used as a "stop early" signal should the action choose to
     *      listen to and act on it.
     *
     * @par
     *      Note as well that it is possible to mark that this action should not
     *      be added to the tracked history, or that the action cannot be undone
     *      however it isn't advisable to do this unless the action specifically
     *      either does not affect the affect the stored project state (such as
     *      saving), or where the history will no longer matter (such as loading
     *      or creating a new project).
     */
    class IAction {
        public:
            //! An alias type to represent the current state of this Action
            using ActionState = uint32_t;

            using Callback = std::function<bool(ActionState)>;

            //! Default callback. Returns true
            static bool _(ActionState) { return true; }

            IAction() = default;
            virtual ~IAction() = default;

            virtual bool doAction(const Callback& = _) = 0;
            virtual bool undoAction(const Callback& = _) = 0;

            virtual bool canBeUndone() const { return true; }
    };
}

#endif

