#ifndef MAPDATA_H
# define MAPDATA_H

# include <memory>
# include <utility>

# include "Types.h"

namespace HMDT {
    /**
     * @brief Holds all representations of the map. Note that this object cannot
     *        be copied, and must either be used as-is or as a shared_ptr
     */
    class MapData {
        public:
            using MapType = std::weak_ptr<uint8_t[]>;
            using ConstMapType = std::weak_ptr<const uint8_t[]>;

            using MapType32 = std::weak_ptr<uint32_t[]>;
            using ConstMapType32 = std::weak_ptr<const uint32_t[]>;

            using MapTypeUUID = std::weak_ptr<UUID[]>;
            using ConstMapTypeUUID = std::weak_ptr<const UUID[]>;

            MapData();
            MapData(uint32_t, uint32_t);
            explicit MapData(const MapData*);

            MapData(MapData&&) = default;

            MapData(const MapData&) = delete;
            MapData& operator=(const MapData&) = delete;

            void close();

            uint32_t getWidth() const;
            uint32_t getHeight() const;
            std::pair<uint32_t, uint32_t> getDimensions() const;

            uint32_t getInputSize() const;
            uint32_t getProvincesSize() const;
            uint32_t getProvinceColorsSize() const;
            uint32_t getProvinceOutlinesSize() const;
            uint32_t getCitiesSize() const;
            uint32_t getMatrixSize() const;
            uint32_t getHeightMapSize() const;
            uint32_t getRiversSize() const;

            bool isClosed() const;

            [[deprecated]] void setLabelMatrix(uint32_t[]);
            [[deprecated]] void setStateIDMatrix(uint32_t[]);

            ////////////////////////////////////////////////////////////////////

            MapType getInput();
            ConstMapType getInput() const;

            MapTypeUUID getProvinces();
            ConstMapTypeUUID getProvinces() const;

            MapType getProvinceColors();
            ConstMapType getProvinceColors() const;

            MapType getProvinceOutlines();
            ConstMapType getProvinceOutlines() const;

            MapType getCities();
            ConstMapType getCities() const;

            MapType32 getLabelMatrix();
            ConstMapType32 getLabelMatrix() const;

            MapType32 getStateIDMatrix();
            ConstMapType32 getStateIDMatrix() const;

            uint32_t getStateIDMatrixUpdatedTag() const;

            MapType getHeightMap();
            ConstMapType getHeightMap() const;

            MapType getRivers();
            ConstMapType getRivers() const;

        private:
            using InternalMapType = std::shared_ptr<uint8_t[]>;
            using InternalMapType32 = std::shared_ptr<uint32_t[]>;
            using InternalMapTypeUUID = std::shared_ptr<UUID[]>;

            uint32_t m_width;
            uint32_t m_height;

            InternalMapType m_input;
            InternalMapTypeUUID m_provinces;
            InternalMapType m_province_colors;
            InternalMapType m_province_outlines;
            InternalMapType m_cities;
            InternalMapType32 m_label_matrix;
            InternalMapType32 m_state_id_matrix;
            InternalMapType m_heightmap;
            InternalMapType m_rivers;
            // More map representations as necessary

            bool m_closed;

            uint32_t m_state_id_matrix_updated_tag;

        public:
            void setLabelMatrix(InternalMapType32);
            void setStateIDMatrix(InternalMapType32);
    };
}

#endif

