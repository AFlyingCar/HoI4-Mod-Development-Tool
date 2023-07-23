#ifndef SELECTION_MANAGER_H
# define SELECTION_MANAGER_H

# include <set>
# include <functional>

# include "Types.h"
# include "MapProject.h"

namespace HMDT::GUI {
    /**
     * @brief Final singleton class which manages all selection logic
     */
    class SelectionManager final {
        public:
            /**
             * @brief The action taken for a selection, used for callbacks.
             */
            enum class Action {
                SET,
                ADD,
                REMOVE,
                CLEAR
            };

            static constexpr StateID INVALID_STATE_ID = -1;

            static SelectionManager& getInstance();

            void selectProvince(const ProvinceID&, bool = false);
            void addProvinceSelection(const ProvinceID&, bool = false);
            void removeProvinceSelection(const ProvinceID&, bool = false);
            void clearProvinceSelection(bool = false);

            void selectState(StateID);
            void addStateSelection(StateID);
            void removeStateSelection(StateID);
            void clearStateSelection();

            RefVector<const Province> getSelectedProvinces() const;
            RefVector<Province> getSelectedProvinces();
            const std::set<ProvinceID>& getSelectedProvinceLabels() const;

            RefVector<const State> getSelectedStates() const;
            RefVector<State> getSelectedStates();
            const std::set<uint32_t>& getSelectedStateIDs() const;

            bool isProvinceSelected(const ProvinceID&) const;
            bool isStateSelected(const StateID&) const;

            void setOnSelectProvinceCallback(const std::function<void(const ProvinceID&, Action)>&);
            void setOnSelectStateCallback(const std::function<void(StateID, Action)>&);

            size_t getSelectedProvinceCount() const;
            size_t getSelectedStateCount() const;

            /////////////////////////////////

            void onProjectLoaded();
            void onProjectUnloaded();

        private:
            SelectionManager();

            OptionalReference<Project::IRootMapProject> getCurrentMapProject() const;
            OptionalReference<Project::IRootHistoryProject> getCurrentHistoryProject() const;

            //! The currently selected provinces
            std::set<ProvinceID> m_selected_provinces;

            //! The currently selected states
            std::set<StateID> m_selected_states;

            //! Callback for when a province is selected
            std::function<void(const ProvinceID&, Action)> m_on_province_selected_callback;

            //! Callback for when a province is selected
            std::function<void(StateID, Action)> m_on_state_selected_callback;
    };
}

#endif

