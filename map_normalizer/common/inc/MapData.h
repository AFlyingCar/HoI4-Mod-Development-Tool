#ifndef MAPDATA_H
# define MAPDATA_H

# include <memory>
# include <utility>

namespace MapNormalizer {
    /**
     * @brief Holds all representations of the map. Note that this object cannot
     *        be copied, and must either be used as-is or as a shared_ptr
     */
    class MapData {
        public:
            using MapType = std::weak_ptr<uint8_t[]>;
            using ConstMapType = std::weak_ptr<const uint8_t[]>;

            MapData(uint32_t, uint32_t);
            MapData(MapData&&) = default;

            MapData(const MapData&) = delete;
            MapData& operator=(const MapData&) = delete;

            void close();

            uint32_t getWidth() const;
            uint32_t getHeight() const;
            std::pair<uint32_t, uint32_t> getDimensions() const;

            bool isClosed() const;

            ////////////////////////////////////////////////////////////////////

            MapType getInput();
            ConstMapType getInput() const;

            MapType getProvinces();
            ConstMapType getProvinces() const;

            MapType getProvinceOutlines();
            ConstMapType getProvinceOutlines() const;

        private:
            using InternalMapType = std::shared_ptr<uint8_t[]>;

            uint32_t m_width;
            uint32_t m_height;

            InternalMapType m_input;
            InternalMapType m_provinces;
            InternalMapType m_province_outlines;
            // More map representations as necessary

            bool m_closed;
    };
}

#endif

