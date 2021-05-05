#ifndef MAPPROJECT_H
# define MAPPROJECT_H

# include <string>
# include <filesystem>

# include "ShapeFinder2.h"

# include "Types.h"
# include "BitMap.h"

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
            void setGraphicsData(unsigned char*);
            void setImage(BitMap*);

            BitMap* getImage();
            const BitMap* getImage() const;

            unsigned char* getGraphicsData();
            const unsigned char* getGraphicsData() const;

        protected:
            bool saveShapeLabels(const std::filesystem::path&);
            bool saveProvinceData(const std::filesystem::path&);

            bool loadShapeLabels(const std::filesystem::path&);
            bool loadProvinceData(const std::filesystem::path&);

        private:
            /**
             * @brief A struct which holds information about shape detection
             */
            struct ShapeDetectionInfo {
                ProvinceList provinces;
                BitMap* image = nullptr;
                uint32_t* label_matrix = nullptr;
                uint32_t label_matrix_size = 0;

                // TODO: This should really be a smart pointer
                unsigned char* graphics_data = nullptr;
            } m_shape_detection_info;

            //! The parent project that this MapProject belongs to
            IProject& m_parent_project;
    };
}

#endif

