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
# include "HeightMapProject.h"
# include "RiversProject.h"

namespace HMDT::Project {
    /**
     * @brief Defines a map project for HoI4
     */
    class MapProject: public IRootMapProject {
        public:
            MapProject(IProject&);
            virtual ~MapProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual MaybeVoid export_(const std::filesystem::path&) const noexcept override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;
            virtual bool validateData() override;

            virtual IRootProject& getRootParent() override;
            virtual const IRootProject& getRootParent() const override;

            virtual IRootMapProject& getRootMapParent() override;
            virtual const IRootMapProject& getRootMapParent() const override;

            virtual ProvinceProject& getProvinceProject() noexcept override;
            virtual const ProvinceProject& getProvinceProject() const noexcept override;

            virtual ContinentProject& getContinentProject() noexcept override;
            virtual const ContinentProject& getContinentProject() const noexcept override;

            virtual RiversProject& getRiversProject() noexcept override;
            virtual const RiversProject& getRiversProject() const noexcept override;

            virtual HeightMapProject& getHeightMapProject() noexcept override;
            virtual const HeightMapProject& getHeightMapProject() const noexcept override;

            virtual void moveProvinceToState(ProvinceID, StateID) override;
            virtual void moveProvinceToState(Province&, StateID) override;
            virtual void removeProvinceFromState(Province&, bool = true) override;

            virtual const std::vector<Terrain>& getTerrains() const override;

            virtual void calculateCoastalProvinces(bool = false) override;

            virtual Maybe<std::shared_ptr<Hierarchy::INode>> visit(const std::function<MaybeVoid(Hierarchy::INode&)>&) const noexcept override;

        protected:
            MaybeVoid validateProvinceStateID(StateID, ProvinceID);

        private:
            //! The Provinces project
            ProvinceProject m_provinces_project;

            //! The Continent project
            ContinentProject m_continent_project;

            //! The HeightMap project
            HeightMapProject m_heightmap_project;

            //! The HeightMap project
            RiversProject m_rivers_project;

            //! The shared map data
            std::shared_ptr<MapData> m_map_data;

            //! All terrains defined for this project
            std::vector<Terrain> m_terrains;

            //! The parent project that this MapProject belongs to
            IProject& m_parent_project;
    };
}

#endif

