
#include "gtest/gtest.h"

#include <filesystem>

#include "HoI4Project.h"
#include "Constants.h"
#include "StatusCodes.h"
#include "Logger.h"
#include "ShapeFinder2.h"

#include "TestUtils.h"
#include "TestMocks.h"

TEST(ProjectTests, SimpleHoI4ProjectTest) {
    HMDT::Project::Project hproject;

    HMDT::Version hoi4_version;

    std::filesystem::path root_path;
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;

    ASSERT_EQ(hproject.getPath(), root_path);
    ASSERT_EQ(hproject.getRoot(), root_path);
    ASSERT_EQ(hproject.getMetaRoot(), projmeta_path);
    ASSERT_EQ(hproject.getInputsRoot(), projmeta_path / "inputs");
    ASSERT_EQ(hproject.getMapRoot(), projmeta_path / "map");

    ASSERT_EQ(hproject.getName(), "");
    ASSERT_EQ(hproject.getToolVersion(), HMDT::TOOL_VERSION);
    ASSERT_EQ(hproject.getHoI4Version(), hoi4_version);
    ASSERT_TRUE(hproject.getTags().empty());
    ASSERT_TRUE(hproject.getOverrides().empty());
}

TEST(ProjectTests, LoadHoI4ProjectTest) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto project_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.hoi4proj";
    auto root_path = project_path.parent_path();
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;

    HMDT::Project::Project hproject(project_path);

    ASSERT_STATUS(hproject.load(), HMDT::STATUS_SUCCESS);

    ASSERT_EQ(hproject.getPath(), project_path);
    ASSERT_EQ(hproject.getRoot(), root_path);
    ASSERT_EQ(hproject.getMetaRoot(), projmeta_path);
    ASSERT_EQ(hproject.getInputsRoot(), projmeta_path / "inputs");
    ASSERT_EQ(hproject.getMapRoot(), projmeta_path / "map");

    ASSERT_EQ(hproject.getName(), "simple");
    ASSERT_EQ(hproject.getToolVersion(), HMDT::Version("1.0.4"));
    ASSERT_EQ(hproject.getHoI4Version(), HMDT::Version("1.10.5"));
    ASSERT_TRUE(hproject.getTags().empty());
    ASSERT_TRUE(hproject.getOverrides().empty());

    HMDT::Log::Logger::getInstance().reset();
}

TEST(ProjectTests, CreateHeightMapProject) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bin_path = HMDT::UnitTests::getTestProgramPath() / "bin";
    auto input_heightmap = bin_path / "complex_heightmap.bmp";
    auto root_path = bin_path.parent_path();
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;

    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto save_path = write_base_path / "heightmap_save";
    auto export_path = write_base_path / "heightmap_export";

    if(!std::filesystem::exists(save_path)) {
        TEST_COUT << "Directory " << save_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(save_path));
    }

    if(!std::filesystem::exists(export_path)) {
        TEST_COUT << "Directory " << export_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(export_path));
    }

    // Set up root project without a path, as we aren't going to test it
    //   directly, but all projects still need a parent
    HMDT::Project::Project hproject;

    // Note that we have to set up the MapData's width+height ourselves, since
    //   we are not loading via the MapProject, and that would handle this part
    //   for us during its 'load' function.
    {
        // We are just hardcoding the values here, since the bitmap hasn't been
        //   loaded yet
        uint32_t width = 1024;
        uint32_t height = 1024;

        WRITE_INFO("Setting up MapData with width=", width, ", height=", height);

        auto map_data_ptr = hproject.getMapProject().getMapData();

        map_data_ptr->~MapData();
        new (map_data_ptr.get()) HMDT::MapData(width, height);
    }

    auto& heightmap_project = hproject.getMapProject().getHeightMapProject();

    auto res = heightmap_project.loadFile(input_heightmap);
    ASSERT_SUCCEEDED(res);

    HMDT::Log::Logger::getInstance().reset();
}

TEST(ProjectTests, SaveAndExportHeightMapProject) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bin_path = HMDT::UnitTests::getTestProgramPath() / "bin";
    auto input_heightmap = bin_path / "complex_heightmap.bmp";
    auto root_path = bin_path.parent_path();
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;

    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto save_path = write_base_path / "heightmap_save";
    auto export_path = write_base_path / "heightmap_export";

    if(!std::filesystem::exists(save_path)) {
        TEST_COUT << "Directory " << save_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(save_path));
    }

    if(!std::filesystem::exists(export_path)) {
        TEST_COUT << "Directory " << export_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(export_path));
    }

    // Set up root project without a path, as we aren't going to test it
    //   directly, but all projects still need a parent
    HMDT::Project::Project hproject;

    auto& heightmap_project = hproject.getMapProject().getHeightMapProject();

    // Note that we have to set up the MapData's width+height ourselves, since
    //   we are not loading via the MapProject, and that would handle this part
    //   for us during its 'load' function.
    {
        // We are just hardcoding the values here, since the bitmap hasn't been
        //   loaded yet
        uint32_t width = 1024;
        uint32_t height = 1024;

        WRITE_INFO("Setting up MapData with width=", width, ", height=", height);

        auto map_data_ptr = hproject.getMapProject().getMapData();

        map_data_ptr->~MapData();
        new (map_data_ptr.get()) HMDT::MapData(width, height);
    }

    WRITE_INFO("Creating HeightMapProject from input file.");
    auto res = heightmap_project.loadFile(input_heightmap);
    ASSERT_SUCCEEDED(res);

    WRITE_INFO("Saving HeightMapProject to ", save_path);
    res = heightmap_project.save(save_path);
    ASSERT_SUCCEEDED(res);

    WRITE_INFO("Exporting HeightMapProject to ", export_path);
    res = heightmap_project.export_(export_path);
    ASSERT_SUCCEEDED(res);

    HMDT::Log::Logger::getInstance().reset();
}

TEST(ProjectTests, HeightMapProjectLoadWithNon8BPPImage) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bin_path = HMDT::UnitTests::getTestProgramPath() / "bin";

    // This is the provinces image, which is _not_ 8-bit or greyscale
    auto input_heightmap = bin_path / "complex.bmp";

    auto root_path = bin_path.parent_path();
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;

    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto save_path = write_base_path / "heightmap_non8bpp_save";
    auto export_path = write_base_path / "heightmap_non8bpp_export";

    if(!std::filesystem::exists(save_path)) {
        TEST_COUT << "Directory " << save_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(save_path));
    }

    if(!std::filesystem::exists(export_path)) {
        TEST_COUT << "Directory " << export_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(export_path));
    }

    // Set up root project without a path, as we aren't going to test it
    //   directly, but all projects still need a parent
    HMDT::Project::Project hproject;

    // Note that we have to set up the MapData's width+height ourselves, since
    //   we are not loading via the MapProject, and that would handle this part
    //   for us during its 'load' function.
    {
        // We are just hardcoding the values here, since the bitmap hasn't been
        //   loaded yet
        uint32_t width = 1024;
        uint32_t height = 1024;

        WRITE_INFO("Setting up MapData with width=", width, ", height=", height);

        auto map_data_ptr = hproject.getMapProject().getMapData();

        map_data_ptr->~MapData();
        new (map_data_ptr.get()) HMDT::MapData(width, height);
    }

    auto& heightmap_project = hproject.getMapProject().getHeightMapProject();

    // First test that it fails if the user chooses not to convert the image
    heightmap_project.setPromptCallback(
        [](const std::string& message,
           const std::vector<std::string>& opts,
           const HMDT::Project::IProject::PromptType&)
            -> uint32_t
        {
            WRITE_INFO("Mocking prompt asking: '", message, "': Response=1");
            return 1;
        });
    auto res = heightmap_project.loadFile(input_heightmap);
    ASSERT_STATUS(res, HMDT::STATUS_INVALID_BIT_DEPTH);

    heightmap_project.setPromptCallback(
        [](const std::string& message,
           const std::vector<std::string>& opts,
           const HMDT::Project::IProject::PromptType&)
            -> uint32_t
        {
            WRITE_INFO("Mocking prompt asking: '", message, "': Response=-1");
            return -1;
        });
    res = heightmap_project.loadFile(input_heightmap);
    ASSERT_STATUS(res, HMDT::STATUS_UNEXPECTED_RESPONSE);

    heightmap_project.setPromptCallback(
        [](const std::string& message,
           const std::vector<std::string>& opts,
           const HMDT::Project::IProject::PromptType&)
            -> uint32_t
        {
            WRITE_INFO("Mocking prompt asking: '", message, "': Response=0");
            return 0;
        });
    res = heightmap_project.loadFile(input_heightmap);
    ASSERT_STATUS(res, HMDT::STATUS_SUCCESS);

    res = heightmap_project.save(save_path);
    ASSERT_SUCCEEDED(res);

    res = heightmap_project.export_(export_path);
    ASSERT_SUCCEEDED(res);

    HMDT::Log::Logger::getInstance().reset();
}

TEST(ProjectTests, RiversProjectExportTemplateTest) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bin_path = HMDT::UnitTests::getTestProgramPath() / "bin";
    auto root_path = bin_path.parent_path();
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;
    auto input_provinces_path = bin_path / "complex.bmp";

    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto save_path = write_base_path / "rivers_save";
    auto export_path = write_base_path / "rivers_export";

    if(!std::filesystem::exists(save_path)) {
        TEST_COUT << "Directory " << save_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(save_path));
    }

    if(!std::filesystem::exists(export_path)) {
        TEST_COUT << "Directory " << export_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directories(export_path));
    }

    // Set up root project without a path, as we aren't going to test it
    //   directly, but all projects still need a parent
    HMDT::Project::Project hproject;

    // Note that for this code to work, it expects a ProvinceProject to exist
    //   and be loaded in so make sure to load one in now

    // Note that we have to set up the MapData's width+height ourselves, since
    //   we are not loading via the MapProject, and that would handle this part
    //   for us during its 'load' function.
    {
        // We are just hardcoding the values here, since the bitmap hasn't been
        //   loaded yet
        uint32_t width = 1024;
        uint32_t height = 1024;

        WRITE_INFO("Setting up MapData with width=", width, ", height=", height);

        auto map_data_ptr = hproject.getMapProject().getMapData();

        map_data_ptr->~MapData();
        new (map_data_ptr.get()) HMDT::MapData(width, height);

        // TODO: ShapeFinder hasn't been switched over to the new BitMap2 object
        //   yet, so we need to still use the deprecated one for now
        WRITE_INFO("Loading ", input_provinces_path);
        HMDT::BitMap* complex_bmp = HMDT::readBMP(input_provinces_path);
        ASSERT_NE(complex_bmp, nullptr);

        HMDT::ShapeFinder sf2(complex_bmp, HMDT::UnitTests::GraphicsWorkerMock::getInstance(), map_data_ptr);
        sf2.findAllShapes();

        hproject.getMapProject().getProvinceProject().import(sf2, map_data_ptr);
    }

    auto& rivers_project = hproject.getMapProject().getRiversProject();

    // Test each response to whether to output a template
    rivers_project.setPromptCallback(
        [](const std::string& message,
           const std::vector<std::string>& opts,
           const HMDT::Project::IProject::PromptType&)
            -> uint32_t
        {
            WRITE_INFO("Mocking prompt asking: '", message, "': Response=1");
            return 1;
        });

    auto res = rivers_project.export_(export_path);
    ASSERT_STATUS(res, HMDT::STATUS_NO_DATA_LOADED);

    // Now test the export of a template image
    rivers_project.setPromptCallback(
        [](const std::string& message,
           const std::vector<std::string>& opts,
           const HMDT::Project::IProject::PromptType&)
            -> uint32_t
        {
            WRITE_INFO("Mocking prompt asking: '", message, "': Response=0");
            return 0;
        });

    res = rivers_project.export_(export_path);
    ASSERT_SUCCEEDED(res);

    HMDT::Log::Logger::getInstance().reset();
}

