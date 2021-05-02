#ifndef MAPPROJECT_H
# define MAPPROJECT_H

# include <string>

# include "ShapeFinder2.h"

# include "IProject.h"

namespace MapNormalizer::Project {
    /**
     * @brief Defines a map project for HoI4
     */
    class MapProject: public IProject {
        public:
            MapProject(IProject&);
            virtual ~MapProject();

            virtual bool save(const std::filesystem::path&) override;
            virtual bool load(const std::filesystem::path&) override;

            void setShapeFinder(ShapeFinder&&);

        protected:
            bool saveShapeLabels(const std::filesystem::path&);
            bool saveProvinceData(const std::filesystem::path&);

            bool loadShapeLabels(const std::filesystem::path&);
            bool loadProvinceData(const std::filesystem::path&);

        private:
            std::unique_ptr<ShapeFinder> m_shape_finder;

            BitMap* m_image;

            IProject& m_parent_project;
    };
}

#endif

