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
    class StateProject: public IMapProject {
        public:
            using StateMap = std::map<uint32_t, State>;

            StateProject(MapProject&);
            virtual ~StateProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;

            virtual bool validateData() override;

            virtual IProject& getRootParent() override;
            virtual IMapProject& getRootMapParent() override;

            MaybeVoid validateProvinceStateID(StateID, ProvinceID);

            const StateMap& getStates() const;

            StateID addNewState(const std::vector<uint32_t>&);
            void removeState(StateID);

            bool isValidStateID(StateID) const;

            MaybeRef<const State> getStateForID(StateID) const;
            MaybeRef<State> getStateForID(StateID);

            State& getStateForIterator(StateMap::const_iterator);
            const State& getStateForIterator(StateMap::const_iterator) const;

            void updateStateIDMatrix();

            MaybeVoid addProvinceToState(StateID, ProvinceID);
            MaybeVoid removeProvinceFromState(StateID, ProvinceID);

        protected:
            StateMap& getMutableStates();

        private:
            //! The parent project that this MapProject belongs to
            MapProject& m_parent_project;

            //! All available state ids, which should be used before new ones
            std::queue<StateID> m_available_state_ids;

            //! All states defined for this project
            StateMap m_states;
    };
}

#endif

