
#include "gtest/gtest.h"

#include <iostream>
#include <filesystem>

#include "ShapeFinder2.h"

#include "MapData.h"

#include "TestOverrides.h"
#include "TestUtils.h"
#include "TestMocks.h"

namespace HMDT::UnitTests {
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
    using namespace HMDT::UnitTests;

    SET_PROGRAM_OPTION(quiet, true);

    const InputImageInfo& iii = images.at("simple");

    std::shared_ptr<HMDT::BitMap> image(new HMDT::BitMap);

    ASSERT_NE(HMDT::readBMP(iii.path, image.get()), nullptr);

    std::shared_ptr<HMDT::MapData> map_data(new HMDT::MapData(image->info_header.width,
                                                              image->info_header.height));

    ShapeFinderMock finder(image.get(), GraphicsWorkerMock::getInstance(), map_data);

    auto border_pixel_count = finder.pass1();

    finder.outputStage(getTestProgramPath() / "simple.pass1.bmp");

    ASSERT_EQ(border_pixel_count, iii.num_border_pixel);
}

TEST(ShapeFinderTests, TestDetectedShapeCount) {
    using namespace HMDT::UnitTests;

    SET_PROGRAM_OPTION(quiet, true);

    const InputImageInfo& iii = images.at("simple");

    std::shared_ptr<HMDT::BitMap> image(new HMDT::BitMap);

    ASSERT_NE(HMDT::readBMP(iii.path, image.get()), nullptr);

    std::shared_ptr<HMDT::MapData> map_data(new HMDT::MapData(image->info_header.width,
                                                              image->info_header.height));

    ShapeFinderMock finder(image.get(), GraphicsWorkerMock::getInstance(), map_data);

    auto&& shapes = finder.findAllShapes();

    ASSERT_EQ(shapes.size(), iii.num_shapes);

    // Verify that there is exactly one label per shape
    std::set<uint32_t> colors;
    std::for_each(map_data->getLabelMatrix().lock().get(),
                  map_data->getLabelMatrix().lock().get() + map_data->getMatrixSize(),
                  [&colors](uint32_t c) { colors.insert(c); });

    ASSERT_EQ(colors.size(), iii.num_shapes);
}

