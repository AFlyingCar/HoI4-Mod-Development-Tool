
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <filesystem>
#include <cstring>
#include <fstream>

#include "HoI4Project.h"
#include "Constants.h"
#include "StatusCodes.h"
#include "Logger.h"
#include "ShapeFinder2.h"
#include "Util.h"

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
    auto tree_result = prov_project.genProvinceChildTree(prov5.id);
    ASSERT_SUCCEEDED(tree_result);
    WRITE_INFO("\n", *tree_result);

    // Attempt to unmerge prov3 from its parent (unmerge leaf province).
    result = prov_project.unmergeProvince(prov3.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov3.parent_id, HMDT::INVALID_PROVINCE);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre(prov1.id, prov5.id));
    ASSERT_THAT(prov5.children, ::testing::UnorderedElementsAre(prov4.id));

    // Attempt to unmerge prov5 (node in middle of tree, has 1 child)
    //   prov5 should no longer have a parent or children, and all of its old
    //   children should now be children of prov2
    result = prov_project.unmergeProvince(prov5.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov5.parent_id, HMDT::INVALID_PROVINCE);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre(prov1.id, prov4.id));
    ASSERT_THAT(prov5.children, ::testing::UnorderedElementsAre());

    // Attempt to unmerge prov2 (root node)
    //   prov2 should no longer have a parent or children, and all of its old
    //   children should now be children of a different child
    result = prov_project.unmergeProvince(prov2.id);
    ASSERT_SUCCEEDED(result);
    ASSERT_EQ(prov2.parent_id, HMDT::INVALID_PROVINCE);
    ASSERT_THAT(prov2.children, ::testing::UnorderedElementsAre());
    ASSERT_THAT(prov1.children, ::testing::UnorderedElementsAre(prov4.id));

    HMDT::Log::Logger::getInstance().reset();
}

