#ifndef IGRAPHICS_WORKER
# define IGRAPHICS_WORKER

# include <cstdint>

namespace MapNormalizer {
    struct Color;
    struct Rectangle;

    class IGraphicsWorker {
        public:
            virtual ~IGraphicsWorker() = default;

            virtual void writeDebugColor(uint32_t, uint32_t, const Color&) = 0;
            virtual void updateCallback(const Rectangle&) = 0;
    };
}

#endif

