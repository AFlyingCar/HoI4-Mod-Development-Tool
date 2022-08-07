#ifndef PROVINCE_PROJECT_H
# define PROVINCE_PROJECT_H

# include "IProject.h"
# include "Types.h"

namespace HMDT::Project {
    /**
     * @brief Defines a province project for HoI4
     */
    class ProvinceProject: public IMapProject, public virtual IProvinceProject {
        public:
            ProvinceProject(IMapProject&);
            virtual ~ProvinceProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;

            virtual bool validateData() override;

            virtual IProject& getRootParent() override;
            virtual IMapProject& getRootMapParent() override;

            virtual ProvinceList& getProvinces() override;
            virtual const ProvinceList& getProvinces() const override;

            virtual ProvinceDataPtr getPreviewData(ProvinceID) override;
            virtual ProvinceDataPtr getPreviewData(const Province*) override;

            void buildProvinceOutlines();
        protected:
            MaybeVoid saveShapeLabels(const std::filesystem::path&);
            MaybeVoid saveProvinceData(const std::filesystem::path&);

            MaybeVoid loadShapeLabels(const std::filesystem::path&);
            MaybeVoid loadProvinceData(const std::filesystem::path&);

            void buildGraphicsData();

        private:
            void buildProvinceCache(const Province*);

            //! The parent project that this MapProject belongs to
            IMapProject& m_parent_project;

            //! List of all provinces
            ProvinceList m_provinces;

            /**
             * @brief A cache of province previews
             * @details Note: We use nlohmann::fifo_map for this so that we can
             *          do hash-based lookup while still retaining FIFO access.
             *          This is so that the least accessed (first in) can get
             *          garbage collected and cleaned out
             */
            nlohmann::fifo_map<ProvinceID, ProvinceDataPtr> m_data_cache;
    };
}

#endif

