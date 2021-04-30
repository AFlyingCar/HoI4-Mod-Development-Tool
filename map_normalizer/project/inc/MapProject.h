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
            MapProject();
            virtual ~MapProject();

            virtual bool save(const std::filesystem::path&) override;
            virtual bool load(const std::filesystem::path&) override;

            void setShapeFinder(ShapeFinder&&);

            // TODO: Provinces, Rivers, etc...
        private:
            ShapeFinder m_shape_finder;
    };
}

#endif

