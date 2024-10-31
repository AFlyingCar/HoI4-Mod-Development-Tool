#ifndef CREATEREMOVESTATEACTION_H
# define CREATEREMOVESTATEACTION_H

# include <string>
# include <optional>
# include <vector>

# include "IProject.h"

# include "IAction.h"

namespace HMDT::Action {
    /**
     * @brief An action that creates or removes a state
     */
    class CreateRemoveStateAction: public Action::IAction {
        public:
            using OnValueChangedCallback = std::function<bool(const StateID&)>;
            using OnValueRemovedCallback = std::function<bool(const State&)>;

            /**
             * @brief The type of operation this action is doing
             */
            enum class Type {
                CREATE,
                REMOVE
            };

            CreateRemoveStateAction(Project::IRootHistoryProject&,
                                    const std::vector<ProvinceID>&);
            CreateRemoveStateAction(Project::IRootHistoryProject&,
                                    const StateID&);

            virtual bool doAction(const Callback& = _) override;
            virtual bool undoAction(const Callback& = _) override;

            CreateRemoveStateAction& onValueChanged(const OnValueChangedCallback&) noexcept;
            CreateRemoveStateAction& onCreate(const OnValueChangedCallback&) noexcept;
            CreateRemoveStateAction& onRemove(const OnValueRemovedCallback&) noexcept;

        protected:
            bool create();
            bool remove();

        private:
            // TODO: This is potentially dangerous to hold onto, we should
            //   instead try to find an elegant solution for getting the current
            //   map project, as it is potentially possible for the map project
            //   to go out of scope before this action is destroyed
            Project::IRootHistoryProject& m_history_project;
            std::vector<ProvinceID> m_provinces;
            StateID m_state_id;
            Type m_type;

            OnValueChangedCallback m_on_value_changed_callback;
            OnValueChangedCallback m_on_create_callback;
            OnValueRemovedCallback m_on_remove_callback;
    };
}

#endif

