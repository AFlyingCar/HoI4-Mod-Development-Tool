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
    class MapProject;

    /**
     * @brief Defines a province project for HoI4
     */
    class StateProject: public IStateProject {
        public:
            struct Token {
                private:
                    Token() = default;

                    friend MapProject;
            };

            StateProject(IRootMapProject&);
            virtual ~StateProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual MaybeVoid export_(const std::filesystem::path&) const noexcept override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;

            virtual bool validateData() override;

            virtual IRootProject& getRootParent() override;
            virtual IRootMapProject& getRootMapParent() override;

            MaybeVoid validateProvinceStateID(StateID, ProvinceID);

            virtual const StateMap& getStates() const override;

            virtual StateID addNewState(const std::vector<uint32_t>&) override;
            virtual void removeState(StateID) override;

            State& getStateForIterator(StateMap::const_iterator);
            const State& getStateForIterator(StateMap::const_iterator) const;

            void updateStateIDMatrix();

            MaybeVoid addProvinceToState(StateID, ProvinceID);
            MaybeVoid removeProvinceFromState(StateID, ProvinceID);

            StateMap& getStateMap(Token) { return getStateMap(); };
        protected:
            virtual StateMap& getStateMap() override;

        private:
            //! The parent project that this MapProject belongs to
            IRootMapProject& m_parent_project;

            //! All available state ids, which should be used before new ones
            std::queue<StateID> m_available_state_ids;

            //! All states defined for this project
            StateMap m_states;
    };
}

#endif

