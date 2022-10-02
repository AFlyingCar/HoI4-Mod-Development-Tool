#ifndef TEST_MOCKS_H
# define TEST_MOCKS_H

# include "ShapeFinder2.h"

namespace HMDT::UnitTests {
    class GraphicsWorkerMock: public IGraphicsWorker {
        public:
            virtual ~GraphicsWorkerMock() = default;

            virtual void writeDebugColor(uint32_t, uint32_t, const Color&) { }
            virtual void updateCallback(const Rectangle&) { }

            static GraphicsWorkerMock& getInstance() {
                static GraphicsWorkerMock instance;

                return instance;
            }
    };

    class ShapeFinderMock: public ShapeFinder {
        public:
            using ShapeFinder::ShapeFinder;

            using ShapeFinder::pass1;
            using ShapeFinder::outputStage;
    };
}

#endif

