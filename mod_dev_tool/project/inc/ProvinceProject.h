#ifndef PROVINCE_PROJECT_H
# define PROVINCE_PROJECT_H

# include "IProject.h"
# include "Types.h"

namespace HMDT::Project {
    /**
     * @brief Defines a province project for HoI4
     */
    class ProvinceProject: public IMapProject {
        public:
            ProvinceProject(IMapProject&);
            virtual ~ProvinceProject();

            virtual MaybeVoid save(const std::filesystem::path&) override;
            virtual MaybeVoid load(const std::filesystem::path&) override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;

            virtual bool validateData() override;

            ProvinceList& getProvinces();
            const ProvinceList& getProvinces() const;

        protected:
            MaybeVoid saveShapeLabels(const std::filesystem::path&);
            MaybeVoid saveProvinceData(const std::filesystem::path&);

            MaybeVoid loadShapeLabels(const std::filesystem::path&);
            MaybeVoid loadProvinceData(const std::filesystem::path&);

        private:
            //! The parent project that this MapProject belongs to
            IMapProject& m_parent_project;

            //! List of all provinces
            ProvinceList m_provinces;
    };
}

#endif

