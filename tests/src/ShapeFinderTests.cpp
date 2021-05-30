
#include "gtest/gtest.h"

#include <iostream>
#include <filesystem>

#include "ShapeFinder2.h"

#include "TestOverrides.h"
#include "TestUtils.h"

namespace MapNormalizer::UnitTests {
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

    struct InputImageInfo {
        std::filesystem::path path;
        uint32_t num_border_pixel;
        uint32_t num_shapes;
    };

    //! All test images and meta information about them for tests
    const std::map<std::string, InputImageInfo> images = {
        { "simple", { getTestProgramPath() / "bin" / "simple.bmp", 3631, 22 } }
    };
}

TEST(ShapeFinderTests, TestPass1BorderCount) {
    using namespace MapNormalizer::UnitTests;

    SET_PROGRAM_OPTION(quiet, true);

    const InputImageInfo& iii = images.at("simple");

    auto image = MapNormalizer::readBMP(iii.path);
    ASSERT_NE(image, nullptr);

    ShapeFinderMock finder(image, GraphicsWorkerMock::getInstance());

    auto border_pixel_count = finder.pass1();

    finder.outputStage(getTestProgramPath() / "simple.pass1.bmp");

    ASSERT_EQ(border_pixel_count, iii.num_border_pixel);
}

