/**
 * @file GraphicalDebugger.h
 *
 * @brief Defines various functions for the graphical debugger.
 */

#ifndef GRAPHICALDEBUGGER_H
# define GRAPHICALDEBUGGER_H

# include <functional>
# include <memory>

# include "IGraphicsWorker.h"
# include "MapData.h"
# include "Types.h"

namespace HMDT {
    void writeDebugColor(uint32_t, uint32_t, Color);

    class GraphicsWorker: public IGraphicsWorker {
        public:
            using UpdateCallback = std::function<void(const Rectangle&)>;

            static GraphicsWorker& getInstance();

            void init(std::shared_ptr<const MapData>);

            void resetDebugData();
            void resetDebugDataAt(const Point2D&);

            std::shared_ptr<const MapData> getMapData() const;

            const UpdateCallback& getWriteCallback() const;
            void setWriteCallback(const UpdateCallback&);
            void resetWriteCallback();

            virtual void writeDebugColor(uint32_t, uint32_t, const Color&) override;
            virtual void updateCallback(const Rectangle&) override;

        private:
            GraphicsWorker() = default;

            std::unique_ptr<unsigned char[]> m_debug_data;
            std::shared_ptr<const MapData> m_map_data;

            UpdateCallback m_write_callback = [](auto) { };
    };

    void checkForPause();
}

#endif

