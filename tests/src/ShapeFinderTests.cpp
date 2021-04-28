
#include "gtest/gtest.h"

#include <iostream>

#include "ShapeFinder2.h"

#include "TestOverrides.h"

namespace {
    class ShapeFinderMock: public MapNormalizer::ShapeFinder {
        public:
            using MapNormalizer::ShapeFinder::ShapeFinder;

            using MapNormalizer::ShapeFinder::pass1;
            using MapNormalizer::ShapeFinder::outputStage;
    };

    struct InputImageInfo {
        const char* path;
        uint32_t num_border_pixel;
        uint32_t num_shapes;
    };

    //! All test images and meta information about them for tests
    const std::map<std::string, InputImageInfo> images = {
        { "./bin/simple.bmp", { "./bin/simple.bmp", 3631, 22 } }
    };
}

TEST(ShapeFinderTests, TestPass1BorderCount) {
    SET_PROGRAM_OPTION(quiet, true);

    const InputImageInfo& iii = images.at("./bin/simple.bmp");

    auto image = MapNormalizer::readBMP(iii.path);

    ShapeFinderMock finder(image);

    auto border_pixel_count = finder.pass1();

    finder.outputStage("./simple.pass1.bmp");

    ASSERT_EQ(border_pixel_count, iii.num_border_pixel);
}

