#ifndef STATE_PROJECT_H
# define STATE_PROJECT_H

# include <queue>
# include <map>
# include <memory>
# include <filesystem>

# include "IProject.h"
# include "Types.h"
# include "Maybe.h"

namespace HMDT::Project {
    /**
     * @brief Defines a province project for HoI4
     */
    class StateProject: public IStateProject {
        public:
            struct Token {
                private:
                    Token() = default;

                    friend IRootHistoryProject;
            };

            StateProject(IRootHistoryProject&);
            virtual ~StateProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual MaybeVoid export_(const std::filesystem::path&) const noexcept override;

            std::shared_ptr<MapData> getMapData();
            const std::shared_ptr<MapData> getMapData() const;

            virtual bool validateData() override;

            virtual IRootProject& getRootParent() override;
            virtual const IRootProject& getRootParent() const override;

            virtual IRootHistoryProject& getRootHistoryParent() noexcept override;
            virtual const IRootHistoryProject& getRootHistoryParent() const noexcept override;

            virtual const StateMap& getStates() const override;

            virtual StateID addNewState(const std::vector<ProvinceID>&) override;
            virtual MaybeVoid removeState(StateID) noexcept override;

            virtual State& getStateForIterator(StateMap::const_iterator) override;
            virtual const State& getStateForIterator(StateMap::const_iterator) const override;

            virtual void updateStateIDMatrix() override;

            virtual MaybeVoid addProvinceToState(StateID, ProvinceID) override;
            virtual MaybeVoid removeProvinceFromState(StateID, ProvinceID) override;

            StateMap& getStateMap(Token) { return getStateMap(); };
        protected:
            virtual StateMap& getStateMap() override;

        private:
            //! The parent project that this HistoryProject belongs to
            IRootHistoryProject& m_parent_project;

            //! All available state ids, which should be used before new ones
            std::queue<StateID> m_available_state_ids;

            //! All states defined for this project
            StateMap m_states;
    };
}

#endif

