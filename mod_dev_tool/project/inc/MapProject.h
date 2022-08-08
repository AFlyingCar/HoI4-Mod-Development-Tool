#ifndef MAPPROJECT_H
# define MAPPROJECT_H

# include <set>
# include <vector>
# include <string>
# include <filesystem>

# include "ShapeFinder2.h"

# include "Types.h"
# include "BitMap.h"
# include "MapData.h"

# include "Terrain.h"

# include "IProject.h"
# include "ProvinceProject.h"
# include "StateProject.h"
# include "ContinentProject.h"

namespace HMDT::Project {
    /**
     * @brief Defines a map project for HoI4
     */
    class MapProject: public IMapProject,
                      public virtual IProvinceProject,
                      public virtual IStateProject,
                      public virtual IContinentProject
    {
        public:
            MapProject(IProject&);
            virtual ~MapProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;
            virtual bool validateData() override;

            virtual IRootProject& getRootParent() override;
            virtual IMapProject& getRootMapParent() override;

            ProvinceProject& getProvinceProject();
            const ProvinceProject& getProvinceProject() const;

            StateProject& getStateProject();
            const StateProject& getStateProject() const;

            virtual const ContinentSet& getContinentList() const override;
            virtual const StateMap& getStates() const override;

            void moveProvinceToState(uint32_t, StateID);
            void moveProvinceToState(Province&, StateID);
            void removeProvinceFromState(Province&, bool = true);

            const std::vector<Terrain>& getTerrains() const;

            virtual ProvinceDataPtr getPreviewData(ProvinceID) override;
            virtual ProvinceDataPtr getPreviewData(const Province*) override;

            virtual ProvinceList& getProvinces() override;
            virtual const ProvinceList& getProvinces() const override;

            void calculateCoastalProvinces(bool = false);

        private:
            virtual ContinentSet& getContinents() override;
            virtual StateMap& getStateMap() override;

            //! The Provinces project
            ProvinceProject m_provinces_project;

            //! The State project
            StateProject m_state_project;

            //! The Continent project
            ContinentProject m_continent_project;

            //! The shared map data
            std::shared_ptr<MapData> m_map_data;

            //! All terrains defined for this project
            std::vector<Terrain> m_terrains;

            //! The parent project that this MapProject belongs to
            IProject& m_parent_project;
    };
}

#endif

