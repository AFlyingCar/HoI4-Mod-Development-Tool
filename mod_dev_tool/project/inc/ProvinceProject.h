#ifndef PROVINCE_PROJECT_H
# define PROVINCE_PROJECT_H

# include "IProject.h"
# include "Types.h"

namespace HMDT::Project {
    /**
     * @brief Defines a province project for HoI4
     */
    class ProvinceProject: public IProvinceProject {
        public:
            ProvinceProject(IRootMapProject&);
            virtual ~ProvinceProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual MaybeVoid export_(const std::filesystem::path&) const noexcept override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;

            virtual bool validateData() override;

            virtual IRootProject& getRootParent() override;
            virtual const IRootProject& getRootParent() const override;

            virtual IRootMapProject& getRootMapParent() override;
            virtual const IRootMapProject& getRootMapParent() const override;

            virtual ProvinceList& getProvinces() override;
            virtual const ProvinceList& getProvinces() const override;

            virtual ProvinceDataPtr getPreviewData(ProvinceID) override;
            virtual ProvinceDataPtr getPreviewData(const Province*) override;

            virtual const std::unordered_map<uint32_t, UUID>& getOldIDToUUIDMap() const noexcept override;

            void buildProvinceOutlines();
        protected:
            MaybeVoid saveShapeLabels(const std::filesystem::path&);
            MaybeVoid saveProvinceData(const std::filesystem::path&, bool = false) const noexcept;

            MaybeVoid loadShapeLabels(const std::filesystem::path&);
            MaybeVoid loadShapeLabels2(const std::filesystem::path&);
            MaybeVoid loadProvinceData(const std::filesystem::path&);
            MaybeVoid loadProvinceData2(const std::filesystem::path&);

            void buildGraphicsData();

        private:
            void buildProvinceCache(const Province*);

            //! The parent project that this MapProject belongs to
            IRootMapProject& m_parent_project;

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

            std::unordered_map<uint32_t, UUID> m_oldid_to_uuid;
    };
}

#endif

