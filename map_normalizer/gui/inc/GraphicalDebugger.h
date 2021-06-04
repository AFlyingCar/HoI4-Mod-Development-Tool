/**
 * @file GraphicalDebugger.h
 *
 * @brief Defines various functions for the graphical debugger.
 */

#ifndef GRAPHICALDEBUGGER_H
# define GRAPHICALDEBUGGER_H

# include <functional>

# include "IGraphicsWorker.h"
# include "BitMap.h"
# include "Types.h"

namespace MapNormalizer {
    void writeDebugColor(uint32_t, uint32_t, Color);

    class GraphicsWorker: public IGraphicsWorker {
        public:
            using UpdateCallback = std::function<void(const Rectangle&)>;

            static GraphicsWorker& getInstance();

            void init(const BitMap*, unsigned char*);

            void resetDebugData();
            void resetDebugDataAt(const Point2D&);

            const unsigned char* getDebugData() const;
            const BitMap* getImage() const;

            const UpdateCallback& getWriteCallback() const;
            void setWriteCallback(const UpdateCallback&);
            void resetWriteCallback();

            virtual void writeDebugColor(uint32_t, uint32_t, const Color&) override;
            virtual void updateCallback(const Rectangle&) override;

        private:
            GraphicsWorker() = default;

            const BitMap* m_image = nullptr;
            unsigned char* m_debug_data = nullptr;

            UpdateCallback m_write_callback = [](auto) { };
    };

    void checkForPause();
}

#endif

