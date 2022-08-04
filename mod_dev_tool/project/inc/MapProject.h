#ifndef MAPPROJECT_H
# define MAPPROJECT_H

# include <set>
# include <vector>
# include <string>
# include <filesystem>

# include "fifo_map.hpp"

# include "ShapeFinder2.h"

# include "Types.h"
# include "BitMap.h"
# include "MapData.h"

# include "Terrain.h"

# include "IProject.h"
# include "ProvinceProject.h"
# include "StateProject.h"

namespace HMDT::Project {
    /**
     * @brief Defines a map project for HoI4
     */
    class MapProject: public IMapProject {
        public:
            using ProvinceDataPtr = std::shared_ptr<unsigned char[]>;

            MapProject(IProject&);
            virtual ~MapProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;
            virtual bool validateData() override;

            virtual IProject& getRootParent() override;
            virtual IMapProject& getRootMapParent() override;

            ProvinceProject& getProvinceProject();
            const ProvinceProject& getProvinceProject() const;

            StateProject& getStateProject();
            const StateProject& getStateProject() const;

            const uint32_t* getLabelMatrix() const;

            bool isValidProvinceLabel(uint32_t) const;
            bool isValidStateID(StateID) const;

            const Province& getProvinceForLabel(uint32_t) const;
            Province& getProvinceForLabel(uint32_t);

            const State& getStateForID(StateID) const;
            State& getStateForID(StateID);

            const std::set<std::string>& getContinentList() const;

            void addNewContinent(const std::string&);
            void removeContinent(const std::string&);
            bool doesContinentExist(const std::string&) const;

            void moveProvinceToState(uint32_t, StateID);
            void moveProvinceToState(Province&, StateID);
            void removeProvinceFromState(Province&, bool = true);

            const std::vector<Terrain>& getTerrains() const;

            ProvinceDataPtr getPreviewData(ProvinceID);
            ProvinceDataPtr getPreviewData(const Province*);

            ProvinceList& getProvinces();
            const ProvinceList& getProvinces() const;

            const std::map<uint32_t, State>& getStates() const;

            void calculateCoastalProvinces(bool = false);

        protected:
            MaybeVoid saveContinentData(const std::filesystem::path&);

            MaybeVoid loadShapeLabels(const std::filesystem::path&);
            MaybeVoid loadProvinceData(const std::filesystem::path&);
            MaybeVoid loadContinentData(const std::filesystem::path&);

        private:
            void buildProvinceCache(const Province*);
            void buildProvinceOutlines();

            //! The Provinces project
            ProvinceProject m_provinces_project;

            //! The State project
            StateProject m_state_project;

            //! The shared map data
            std::shared_ptr<MapData> m_map_data;

            /**
             * @brief A cache of province previews
             * @details Note: We use nlohmann::fifo_map for this so that we can
             *          do hash-based lookup while still retaining FIFO access.
             *          This is so that the least accessed (first in) can get
             *          garbage collected and cleaned out
             */
            nlohmann::fifo_map<ProvinceID, ProvinceDataPtr> m_data_cache;

            //! All continents defined for this project
            std::set<std::string> m_continents;

            //! All terrains defined for this project
            std::vector<Terrain> m_terrains;

            //! The parent project that this MapProject belongs to
            IProject& m_parent_project;
    };
}

#endif

