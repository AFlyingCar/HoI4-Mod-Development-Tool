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

            virtual bool save(const std::filesystem::path&,
                              std::error_code& = last_error) override;
            virtual bool load(const std::filesystem::path&,
                              std::error_code& = last_error) override;
            virtual void import(const ShapeFinder&, std::shared_ptr<MapData>) override;

            virtual std::shared_ptr<MapData> getMapData() override;
            virtual const std::shared_ptr<MapData> getMapData() const override;

            ProvinceList& getProvinces();
            const ProvinceList& getProvinces() const;

        protected:
            bool saveShapeLabels(const std::filesystem::path&,
                                 std::error_code&);
            bool saveProvinceData(const std::filesystem::path&,
                                 std::error_code&);

            bool loadShapeLabels(const std::filesystem::path&,
                                 std::error_code&);
            bool loadProvinceData(const std::filesystem::path&,
                                 std::error_code&);

        private:
            //! The parent project that this MapProject belongs to
            IMapProject& m_parent_project;

            //! List of all provinces
            ProvinceList m_provinces;
    };
}

#endif

