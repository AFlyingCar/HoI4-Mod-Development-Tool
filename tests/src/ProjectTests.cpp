
#include "gtest/gtest.h"

#include <filesystem>

#include "HoI4Project.h"
#include "Constants.h"

#include "TestUtils.h"

TEST(ProjectTests, SimpleHoI4ProjectTest) {
    MapNormalizer::Project::Project hproject;

    MapNormalizer::Version hoi4_version;

    std::filesystem::path root_path;
    auto projmeta_path = root_path / MapNormalizer::PROJ_META_FOLDER;

    ASSERT_EQ(hproject.getPath(), root_path);
    ASSERT_EQ(hproject.getRoot(), root_path);
    ASSERT_EQ(hproject.getMetaRoot(), projmeta_path);
    ASSERT_EQ(hproject.getInputsRoot(), projmeta_path / "inputs");
    ASSERT_EQ(hproject.getMapRoot(), projmeta_path / "map");

    ASSERT_EQ(hproject.getName(), "");
    ASSERT_EQ(hproject.getToolVersion(), MapNormalizer::TOOL_VERSION);
    ASSERT_EQ(hproject.getHoI4Version(), hoi4_version);
    ASSERT_TRUE(hproject.getTags().empty());
    ASSERT_TRUE(hproject.getOverrides().empty());
}

TEST(ProjectTests, LoadHoI4ProjectTest) {
    auto project_path = MapNormalizer::UnitTests::getTestProgramPath() / "bin" / "simple.hoi4proj";
    auto root_path = project_path.parent_path();
    auto projmeta_path = root_path / MapNormalizer::PROJ_META_FOLDER;

    MapNormalizer::Project::Project hproject(project_path);

    ASSERT_TRUE(hproject.load());

    ASSERT_EQ(hproject.getPath(), project_path);
    ASSERT_EQ(hproject.getRoot(), root_path);
    ASSERT_EQ(hproject.getMetaRoot(), projmeta_path);
    ASSERT_EQ(hproject.getInputsRoot(), projmeta_path / "inputs");
    ASSERT_EQ(hproject.getMapRoot(), projmeta_path / "map");

    ASSERT_EQ(hproject.getName(), "simple");
    ASSERT_EQ(hproject.getToolVersion(), MapNormalizer::Version("1.0.4"));
    ASSERT_EQ(hproject.getHoI4Version(), MapNormalizer::Version("1.10.5"));
    ASSERT_TRUE(hproject.getTags().empty());
    ASSERT_TRUE(hproject.getOverrides().empty());
}

