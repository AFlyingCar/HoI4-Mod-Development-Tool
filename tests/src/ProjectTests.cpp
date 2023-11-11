
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <filesystem>
#include <cstring>
#include <fstream>
#include <stack>
#include <vector>

#include "HoI4Project.h"
#include "Constants.h"
#include "StatusCodes.h"
#include "Logger.h"
#include "ShapeFinder2.h"
#include "Util.h"
#include "ProjectNode.h"
#include "LinkNode.h"

#include "TestUtils.h"
#include "TestMocks.h"
#include "TestOverrides.h"

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
    auto input_provinces_path = bin_path / "complex.bmp";

    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto export_path = write_base_path / "rivers_export";

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

TEST(ProjectTests, ConvertV1ProvinceIDsToUUIDTest) {
    SET_PROGRAM_OPTION(debug, true);

    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bin_path = HMDT::UnitTests::getTestProgramPath() / "bin";

    // Load a project file that has a tool version number <= 0.25.0
    auto project_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple2.hoi4proj";
    auto prov_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "map_v1provinceids";
    auto debug_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "test_debug";

    HMDT::UnitTests::HoI4ProjectMock hproject(project_path);
    hproject.setDebugRoot(debug_path);

    // Since we aren't actually loading the project file, we need to make sure
    //   that the tool version is set to something <= 0.25.0
    hproject.setToolVersion(HMDT::Version{"0.24.0"});

    auto& map_project = hproject.getMapProject();
    auto& prov_project = map_project.getProvinceProject();

    // Make sure that we set up the MapData before attempting to load the provinces
    {
        // Hard-code these values since we aren't actually loading from a real
        //   image here
        auto iwidth = 512;
        auto iheight = 512;

        auto map_data = map_project.getMapData();

        // Do a placement new so we keep the same memory location but update all
        //  of the data inside the shared MapData instead, so that all
        //  references are also updated too
        map_data->~MapData();
        new (map_data.get()) HMDT::MapData(iwidth, iheight);
    }

    ASSERT_STATUS(prov_project.load(prov_path), HMDT::STATUS_SUCCESS);

    // TODO: Validate UUIDs are valid

    HMDT::Log::Logger::getInstance().reset();
}

TEST(ProjectTests, ProvinceProjectSaveAndLoad) {
    SET_PROGRAM_OPTION(debug, true);

    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bin_path = HMDT::UnitTests::getTestProgramPath() / "bin";

    auto input_provinces_path = bin_path / "simple.bmp";

    auto project_path = bin_path / "simple2.hoi4proj";
    auto prov_path = bin_path / "map_province_saveandload";
    auto debug_path = bin_path / "test_debug";

    HMDT::UnitTests::HoI4ProjectMock hproject(project_path);
    hproject.setDebugRoot(debug_path);

    auto& map_project = hproject.getMapProject();
    auto& prov_project = map_project.getProvinceProject();

    auto map_data = map_project.getMapData();

    // Make sure that we set up the MapData before attempting to load the provinces
    {
        // Hard-code these values since we aren't actually loading from a real
        //   image here
        auto iwidth = 512;
        auto iheight = 512;

        // Do a placement new so we keep the same memory location but update all
        //  of the data inside the shared MapData instead, so that all
        //  references are also updated too
        map_data->~MapData();
        new (map_data.get()) HMDT::MapData(iwidth, iheight);
    }

    HMDT::MaybeVoid result = HMDT::STATUS_SUCCESS;

    // First import test data
    {
        // TODO: ShapeFinder hasn't been switched over to the new BitMap2 object
        //   yet, so we need to still use the deprecated one for now
        WRITE_INFO("Loading ", input_provinces_path);
        HMDT::BitMap* complex_bmp = HMDT::readBMP(input_provinces_path);
        ASSERT_NE(complex_bmp, nullptr);

        HMDT::ShapeFinder sf2(complex_bmp, HMDT::UnitTests::GraphicsWorkerMock::getInstance(), map_data);
        sf2.findAllShapes();

        prov_project.import(sf2, map_data);
    }

    std::filesystem::create_directories(prov_path);

    // Make sure that the tool version is higher than 0.25.0
    hproject.setToolVersion(HMDT::Version{"0.25.1"});

    // Save the project
    result = prov_project.save(prov_path);
    ASSERT_SUCCEEDED(result);

    // Save a copy of the map data here before loading so we can compare against
    //   it
    std::unique_ptr<HMDT::UUID[]> prov_data(new HMDT::UUID[map_data->getProvincesSize()]{ HMDT::EMPTY_UUID });
    std::memcpy(prov_data.get(),
                map_data->getProvinces().lock().get(),
                map_data->getProvincesSize() * sizeof(HMDT::UUID));

    // Load in the saved province data manually and verify that it matches what
    //   we initially saved
    WRITE_INFO("Reading saved province data from ",
               prov_path / HMDT::SHAPEDATA_FILENAME);
    if(std::ifstream in(prov_path / HMDT::SHAPEDATA_FILENAME); in) {
        std::unique_ptr<HMDT::UUID[]> temp_data(new HMDT::UUID[map_data->getProvincesSize()]{ HMDT::EMPTY_UUID });

        unsigned char magic[4];
        uint32_t width = 0;
        uint32_t height = 0;

        // Read in the header first
        ASSERT_TRUE(HMDT::safeRead(in, &magic, &width, &height));

        // Verify that the magic bytes are correct
        ASSERT_TRUE(HMDT::UnitTests::dynamicArraysMatch(
                    reinterpret_cast<char*>(magic),
                    HMDT::SHAPEDATA_MAGIC.data(), 4));

        // Verify that the dimensions match
        ASSERT_EQ(width * height, map_data->getMatrixSize());

        in.read(reinterpret_cast<char*>(temp_data.get()),
                map_data->getProvincesSize() * sizeof(HMDT::UUID));

        ASSERT_TRUE(HMDT::UnitTests::dynamicArraysMatch(prov_data.get(),
                                                        temp_data.get(),
                                                        map_data->getProvincesSize()));
    } else {
        WRITE_ERROR("Failed to load province data.");
        ASSERT_TRUE(false);
    }

    // Now load the project back into memory
    result = prov_project.load(prov_path);
    ASSERT_SUCCEEDED(result);

    // Verify that the loaded province data matchces what was saved to disk
    ASSERT_TRUE(std::equal(prov_data.get(),
                           prov_data.get() + map_data->getProvincesSize(),
                           map_data->getProvinces().lock().get()));

    HMDT::Log::Logger::getInstance().reset();
}

TEST(ProjectTests, MergeProvinceTests) {
    SET_PROGRAM_OPTION(debug, true);

    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bin_path = HMDT::UnitTests::getTestProgramPath() / "bin";

    auto input_provinces_path = bin_path / "simple.bmp";

    auto project_path = bin_path / "simple2.hoi4proj";
    auto prov_path = bin_path / "map_province_saveandload";
    auto debug_path = bin_path / "test_debug";

    HMDT::UnitTests::HoI4ProjectMock hproject(project_path);
    hproject.setDebugRoot(debug_path);

    auto& map_project = hproject.getMapProject();
    auto& prov_project = map_project.getProvinceProject();

    auto map_data = map_project.getMapData();

    // Make sure that we set up the MapData before attempting to load the provinces
    {
        // Hard-code these values since we aren't actually loading from a real
        //   image here
        auto iwidth = 512;
        auto iheight = 512;

        // Do a placement new so we keep the same memory location but update all
        //  of the data inside the shared MapData instead, so that all
        //  references are also updated too
        map_data->~MapData();
        new (map_data.get()) HMDT::MapData(iwidth, iheight);
    }

    // First import test data
    {
        // TODO: ShapeFinder hasn't been switched over to the new BitMap2 object
        //   yet, so we need to still use the deprecated one for now
        WRITE_INFO("Loading ", input_provinces_path);
        HMDT::BitMap* complex_bmp = HMDT::readBMP(input_provinces_path);
        ASSERT_NE(complex_bmp, nullptr);

        HMDT::ShapeFinder sf2(complex_bmp, HMDT::UnitTests::GraphicsWorkerMock::getInstance(), map_data);
        sf2.findAllShapes();

        prov_project.import(sf2, map_data);
    }

    // Find two unrelated provinces
    const auto& provinces = prov_project.getProvinces();

    auto provinces_it = provinces.begin();

    const auto& prov1 = provinces_it->second;
    provinces_it = ++provinces_it;

    const auto& prov2 = provinces_it->second;
    provinces_it = ++provinces_it;

    WRITE_DEBUG("prov1.id=", prov1.id, ", prov1.parent_id=", prov1.parent_id,
                ", prov2.id=", prov2.id, ", prov2.parent_id=", prov2.parent_id);

    // Attempt to merge two unrelated provinces together
    auto result = prov_project.mergeProvinces(prov1.id, prov2.id);
    ASSERT_SUCCEEDED(result);
    WRITE_DEBUG("prov1.id=", prov1.id, ", prov1.parent_id=", prov1.parent_id,
                ", prov2.id=", prov2.id, ", prov2.parent_id=", prov2.parent_id);
    ASSERT_EQ(prov1.parent_id, prov2.id);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre(prov1.id));

    // Attempt to merge a 3rd province into one that already has a parent
    const auto& prov3 = provinces_it->second;
    provinces_it = ++provinces_it;

    result = prov_project.mergeProvinces(prov3.id, prov1.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov3.parent_id, prov2.id);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre(prov1.id, prov3.id));

    const auto& prov4 = provinces_it->second;
    provinces_it = ++provinces_it;

    const auto& prov5 = provinces_it->second;
    provinces_it = ++provinces_it;

    // Merge 2 more unrelated provinces together for the next test
    result = prov_project.mergeProvinces(prov4.id, prov5.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov4.parent_id, prov5.id);
    ASSERT_THAT(prov5.children, ::testing::UnorderedElementsAre(prov4.id));

    // Attempt to merge two provinces which already have parents together
    // Prov1 parent: prov2
    // Prov2 parent: none
    // Prov3 parent: prov2
    // Prov4 parent: prov5
    result = prov_project.mergeProvinces(prov4.id, prov1.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov4.parent_id, prov5.id);
    ASSERT_EQ(prov5.parent_id, prov2.id); // prov4's parent should now have prov1's parent as its own parent

    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre(prov1.id, prov3.id, prov5.id));
    ASSERT_THAT(prov5.children, ::testing::UnorderedElementsAre(prov4.id));

    // Validate getMergedProvinces
    auto&& prov1_merged = prov_project.getMergedProvinces(prov1.id);
    WRITE_INFO("Got ", prov1_merged.size(), " provinces merged with ", prov1.id);
    ASSERT_THAT(prov1_merged, ::testing::UnorderedElementsAre(prov1.id, prov2.id, prov3.id, prov5.id, prov4.id));

    // Get a province that we have not merged at all and make sure it's the only
    //   thing in the list
    ++provinces_it;
    const auto& unmerged_prov = provinces_it->second;
    auto&& unmerged_mergelist = prov_project.getMergedProvinces(unmerged_prov.id);
    ASSERT_THAT(unmerged_mergelist, ::testing::UnorderedElementsAre(unmerged_prov.id));

    // Get an invalid province ID and make sure that the list generated from that
    //   is empty
    HMDT::ProvinceID invalid_id;
    const auto& invalid_mergelist = prov_project.getMergedProvinces(invalid_id);
    ASSERT_THAT(invalid_mergelist, ::testing::UnorderedElementsAre());

    // Print out the tree for prov5
    // Expected example output:
    // 09cc163a-d185-4aa2-b44b-5edb0cacb202  <-  00000000-0000-0000-0000-000000000000
    // │   ├── 01272dcd-9c3d-4b93-8721-06f7116dbf9d  <-  09cc163a-d185-4aa2-b44b-5edb0cacb202
    // │   │   ├── 09c40c9f-e83f-480d-8ed7-077dd22ad6b3  <-  09cc163a-d185-4aa2-b44b-5edb0cacb202
    // │   │   ├── 0a784764-8c41-4625-a296-5c6deabae5cd  <-  09cc163a-d185-4aa2-b44b-5edb0cacb202
    // │   │   │   └── 0333927b-d4d2-4b67-b2d0-738e4013001f  <-  0a784764-8c41-4625-a296-5c6deabae5cd
    // │   │
    //
    // Where:
    //   prov1=01272dcd-9c3d-4b93-8721-06f7116dbf9d
    //   prov2=09cc163a-d185-4aa2-b44b-5edb0cacb202
    //   prov3=09c40c9f-e83f-480d-8ed7-077dd22ad6b3
    //   prov4=0333927b-d4d2-4b67-b2d0-738e4013001f
    //   prov5=0a784764-8c41-4625-a296-5c6deabae5cd
    auto tree_result = prov_project.genProvinceChildTree(prov5.id);
    ASSERT_SUCCEEDED(tree_result);
    WRITE_INFO("\n", *tree_result);

    // Attempt to unmerge prov3 from its parent (unmerge leaf province).
    // Result should be:
    // prov2
    // ├── prov1
    // │   ├── prov5
    // │   │   └── prov4
    result = prov_project.unmergeProvince(prov3.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov3.parent_id, HMDT::INVALID_PROVINCE);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre(prov1.id, prov5.id));
    ASSERT_THAT(prov5.children, ::testing::UnorderedElementsAre(prov4.id));

    // Attempt to unmerge prov5 (node in middle of tree, has 1 child)
    //   prov5 should no longer have a parent or children, and all of its old
    //   children should now be children of prov2
    // Result should be:
    // prov2
    // ├── prov1
    // └── prov4
    result = prov_project.unmergeProvince(prov5.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov5.parent_id, HMDT::INVALID_PROVINCE);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre(prov1.id, prov4.id));
    ASSERT_THAT(prov5.children, ::testing::UnorderedElementsAre());

    // Attempt to unmerge prov2 (root node)
    //   prov2 should no longer have a parent or children, and all of its old
    //   children should now be children of a different child
    // Result should be either:
    // prov1
    // └── prov4
    // OR
    // prov4
    // └── prov1

    // Print out the tree again
    tree_result = prov_project.genProvinceChildTree(prov5.id);
    ASSERT_SUCCEEDED(tree_result);
    WRITE_INFO("\n", *tree_result);

    result = prov_project.unmergeProvince(prov2.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov2.parent_id, HMDT::INVALID_PROVINCE);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre());

    // Because we don't necessarily know which child will be chosen as the new
    //   parent, test both possibilities (and test to make sure at leats one of
    //   them contains a value)
    ASSERT_THAT(prov1.children,
            ::testing::AnyOf(
                ::testing::UnorderedElementsAre(),
                ::testing::UnorderedElementsAre(prov4.id)));
    ASSERT_THAT(prov4.children,
            ::testing::AnyOf(
                ::testing::UnorderedElementsAre(),
                ::testing::UnorderedElementsAre(prov1.id)));
    // Logical XOR
    ASSERT_TRUE(!prov1.children.empty() != !prov4.children.empty());

    HMDT::Log::Logger::getInstance().reset();
}

TEST(ProjectTests, SimpleHierarchyTest) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto project_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.hoi4proj";
    auto root_path = project_path.parent_path();
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;

    HMDT::Project::Project hproject(project_path);
    ASSERT_STATUS(hproject.load(), HMDT::STATUS_SUCCESS);

    auto maybe_root_node = hproject.visit([](auto&&...){ return HMDT::STATUS_SUCCESS; });
    ASSERT_SUCCEEDED(maybe_root_node);

    auto root_node = *maybe_root_node;

    ASSERT_EQ(root_node->getType(), HMDT::Project::Hierarchy::Node::Type::PROJECT);

    auto project_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::ProjectNode>(root_node);
    ASSERT_NE(project_node, nullptr);

    ASSERT_EQ(project_node->getChildren().size(), 5);

    // Check the various non-project nodes

    auto maybe_name_node = (*project_node)["Name"];
    ASSERT_SUCCEEDED(maybe_name_node);
    ASSERT_EQ((*maybe_name_node)->getType(), HMDT::Project::Hierarchy::Node::Type::PROPERTY);
    auto name_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::IPropertyNode>(*maybe_name_node);
    ASSERT_NE(name_node, nullptr);
    ASSERT_EQ(*name_node, hproject.getName());

    auto maybe_version_node = (*project_node)["HoI4 Version"];
    ASSERT_SUCCEEDED(maybe_version_node);
    ASSERT_EQ((*maybe_version_node)->getType(), HMDT::Project::Hierarchy::Node::Type::PROPERTY);
    auto version_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::IPropertyNode>(*maybe_version_node);
    ASSERT_NE(version_node, nullptr);
    ASSERT_EQ(*version_node, hproject.getHoI4Version());

    auto maybe_tags_node = (*project_node)["Tags"];
    ASSERT_SUCCEEDED(maybe_tags_node);
    ASSERT_EQ((*maybe_tags_node)->getType(), HMDT::Project::Hierarchy::Node::Type::GROUP);
    auto tags_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::IGroupNode>(*maybe_tags_node);
    ASSERT_NE(tags_node, nullptr);
    ASSERT_EQ(tags_node->getChildren().size(), 0);
}

TEST(ProjectTests, SimpleHierarchyIterationTest) {
    // We also want to see log outputs in the test output
    // HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto project_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.hoi4proj";
    auto root_path = project_path.parent_path();
    auto projmeta_path = root_path / HMDT::PROJ_META_FOLDER;

    HMDT::Project::Project hproject(project_path);
    ASSERT_STATUS(hproject.load(), HMDT::STATUS_SUCCESS);


    std::vector<HMDT::ProvinceID> state1_provs;
    std::vector<HMDT::ProvinceID> state2_provs;

    // Load in some province data and create a few states
    {
        // Load in province data
        auto path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.bmp";

        std::shared_ptr<HMDT::BitMap> image(new HMDT::BitMap);

        ASSERT_NE(HMDT::readBMP(path, image.get()), nullptr);

        std::shared_ptr<HMDT::MapData> map_data(new HMDT::MapData(image->info_header.width,
                                                                  image->info_header.height));

        HMDT::ShapeFinder finder(image.get(),
                                 HMDT::UnitTests::GraphicsWorkerMock::getInstance(),
                                 map_data);
        finder.findAllShapes();

        hproject.getMapProject().import(finder, map_data);

        auto& prov_proj = hproject.getMapProject().getProvinceProject();

        // Get some provinces to put into the states
        std::transform(
            prov_proj.getProvinces().begin(),
            std::next(prov_proj.getProvinces().begin(), 5),
            std::back_inserter(state1_provs),
            [](auto&& kv) { return kv.first; }
        );
        std::transform(
            std::next(prov_proj.getProvinces().begin(), 5),
            std::next(prov_proj.getProvinces().begin(), 10),
            std::back_inserter(state2_provs),
            [](auto&& kv) { return kv.first; }
        );

        // Generate a few states
        hproject.getHistoryProject().getStateProject().addNewState(state1_provs);
        hproject.getHistoryProject().getStateProject().addNewState(state2_provs);
    }

    auto maybe_root_node = hproject.visit([](auto node) -> HMDT::MaybeVoid {
        return HMDT::STATUS_SUCCESS;
    });
    ASSERT_SUCCEEDED(maybe_root_node);

    auto root_node = *maybe_root_node;

    // Visit the entire tree and resolve all link nodes
    auto result = root_node->visit([&root_node](auto node)
        -> HMDT::MaybeVoid
    {
        if(node->getType() == HMDT::Project::Hierarchy::Node::Type::LINK) {
            WRITE_DEBUG("Resolve link node ", node->getName());
            auto link_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::LinkNode>(node);
            auto result = link_node->resolve(root_node);
            RETURN_IF_ERROR(result);

            if(!link_node->isLinkValid()) {
                WRITE_ERROR("Link resolution succeeded, but the link node is still invalid.");
                RETURN_ERROR(HMDT::STATUS_UNEXPECTED);
            }
        }

        return HMDT::STATUS_SUCCESS;
    });
    ASSERT_SUCCEEDED(result);

    ASSERT_EQ(root_node->getType(), HMDT::Project::Hierarchy::Node::Type::PROJECT);

    auto project_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::ProjectNode>(root_node);
    ASSERT_NE(project_node, nullptr);

    // Debug dump tree. Leave this disabled unless things are _really_ broken
#if 0
    std::stack<uint32_t> indents;
    indents.push(0);
    root_node->visit([&indents](auto node) -> HMDT::MaybeVoid {
        uint32_t indent_lvl = indents.top();
        indents.pop();

        WRITE_DEBUG(std::string(2 * indent_lvl, ' '), std::to_string(*node));

        switch(node->getType()) {
            case HMDT::Project::Hierarchy::Node::Type::GROUP:
            case HMDT::Project::Hierarchy::Node::Type::PROJECT:
            case HMDT::Project::Hierarchy::Node::Type::STATE:
            case HMDT::Project::Hierarchy::Node::Type::PROVINCE:
            {
                auto group_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::IGroupNode>(node);
                for(auto i = 0; i < group_node->getChildren().size(); ++i) {
                    indents.push(indent_lvl + 1);
                }
            }
            default:;
        }

        return HMDT::STATUS_SUCCESS;
    });
#endif

    // Depth-first iteration over the entire tree structure
    std::stack<std::shared_ptr<HMDT::Project::Hierarchy::IGroupNode>> next_groups;
    next_groups.push(project_node);

    uint32_t last_indent = 0;
    std::stack<uint32_t> next_indents;
    next_indents.push(0);
    while(!next_groups.empty()) {
        auto next_group = next_groups.top();
        next_groups.pop();

        // Formatting
        uint32_t indent_level = next_indents.top();
        next_indents.pop();
        std::string indent1(2 * indent_level, ' ');
        std::string indent2(2 * (indent_level + 1), ' ');

        WRITE_INFO(indent1, std::to_string(*next_group), "={");
        for(auto&& [name, child_node] : next_group->getChildren()) {
            switch(child_node->getType()) {
                // If it is a group or group-like, make sure to add it to the
                //   back of the groups queue
                case HMDT::Project::Hierarchy::Node::Type::GROUP:
                case HMDT::Project::Hierarchy::Node::Type::PROJECT:
                case HMDT::Project::Hierarchy::Node::Type::STATE:
                case HMDT::Project::Hierarchy::Node::Type::PROVINCE:
                {
                    next_indents.push(indent_level + 1);
                    auto group_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::IGroupNode>(child_node);
                    ASSERT_NE(group_node, nullptr);
                    next_groups.push(group_node);
                    break;
                }
                case HMDT::Project::Hierarchy::Node::Type::PROPERTY:
                case HMDT::Project::Hierarchy::Node::Type::CONST_PROPERTY:
                {
                    auto prop_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::IPropertyNode>(child_node);
                    ASSERT_NE(prop_node, nullptr);

                    // TODO: Verification?
                    std::string value_string = "<";
                    if(prop_node->hasValue()) {
                        value_string.append("data> [type=");
                        value_string.append(prop_node->getTypeInfo()->name());
                        value_string.append("]");
                    } else {
                        value_string += "null>";
                    }
                    WRITE_INFO(indent2, std::to_string(*prop_node), "=", value_string);

                    break;
                }
                case HMDT::Project::Hierarchy::Node::Type::LINK:
                {
                    auto link_node = std::dynamic_pointer_cast<HMDT::Project::Hierarchy::ILinkNode>(child_node);
                    ASSERT_NE(link_node, nullptr);

                    EXPECT_TRUE(link_node->isLinkValid());

                    // TODO: Verification?
                    std::string linked_node_string = link_node->isLinkValid() ?
                                                     std::to_string(*link_node->getLinkedNode()) :
                                                     std::string("<UNKNOWN>");
                    WRITE_INFO(indent2, std::to_string(*link_node), "->", linked_node_string);

                    break;
                }
            }
        }
        if(next_indents.empty()               ||
           next_indents.top() == indent_level ||
           next_indents.top() == indent_level - 1)
        {
            WRITE_INFO(indent1, "}");
        }
        last_indent = indent_level;
    }

    for(uint32_t i = 0, m=last_indent; i < m; ++i) {
        std::string indent(2 * (last_indent - 1), ' ');
        --last_indent;

        WRITE_INFO(indent, "}");
    }
}

