
#include "PreferencesTests.h"

#include "gtest/gtest.h"

#include "Preferences.h"

#include "TestOverrides.h"
#include "TestUtils.h"

namespace HMDT::UnitTests {
    static auto simple_conf_path = getTestProgramPath() / "bin" / "simple.conf";

    Preferences::SectionMap simple_conf_defaults =
PREF_BEGIN_DEF()
    PREF_BEGIN_DEFINE_SECTION("SimpleSection",)
        PREF_SECTION_DEFINE_PROPERTY(showTitles, true)

        PREF_BEGIN_DEFINE_GROUP("SimpleGroup",)
            PREF_DEFINE_CONFIG("val1", false, , false)
            PREF_DEFINE_CONFIG("val2", 5L, , false)
            PREF_DEFINE_CONFIG("val3", "a fun string", , false)
            PREF_DEFINE_CONFIG("val4", 3.1415, , false)
        PREF_END_DEFINE_GROUP()

        PREF_BEGIN_DEFINE_GROUP("SimpleGroup2",)
            PREF_DEFINE_CONFIG("val2", 23L, , false)
        PREF_END_DEFINE_GROUP()
    PREF_END_DEFINE_SECTION()
PREF_END_DEF();

    // Define a custom operator== for Preferences::Section
    bool compareSections(const Preferences::Section& lhs,
                         const Preferences::Section& rhs)
    {
        // NOTE: Update withh new properties for testing
        bool result = lhs.showTitles == rhs.showTitles;

        return result && lhs.groups.size() == rhs.groups.size() &&
               std::equal(lhs.groups.begin(), lhs.groups.end(),
                          rhs.groups.begin(),
                          [](const auto& a, const auto& b) {
                              return a.first == b.first &&
                                     std::equal(a.second.configs.begin(),
                                                a.second.configs.end(),
                                                b.second.configs.begin(),
                                                [](const auto& a,
                                                   const auto& b)
                                                {
                                                    return a.first == b.first &&
                                                           a.second == b.second;
                                                });
                          });
    }

    void PreferencesTests::SetUpTestSuite() {
        registerTestLogOutputFunction(true, true, true, true);
    }

    void PreferencesTests::TearDownTestSuite() {
        Log::Logger::getInstance().reset();
    }

    void PreferencesTests::SetUp() { }

    void PreferencesTests::TearDown() {
        Preferences::getInstance(false)._reset();
    }

    /**
     * @brief Validate that GetInstance does not actually initialize until after
     *        the path has been set.
     */
    TEST_F(PreferencesTests, GetInstanceDoesNotInitUntilPathSet) {
        ASSERT_FALSE(Preferences::getInstance().isInitialized());

        // Now make sure that it gets initialized after we set the config path
        Preferences::getInstance().setConfigLocation(simple_conf_path);

        ASSERT_TRUE(Preferences::getInstance().isInitialized());
    }

    TEST_F(PreferencesTests, ParseValueTest) {
        std::string path1 = "Hello.World._";
        std::string path2 = "_._._";
        std::string path3 = "Invalid path";
        std::string path4 = "long.invalid.path.here";

        auto path_triple1 = std::make_tuple("Hello", "World", "_");
        auto path_triple2 = std::make_tuple("_", "_", "_");

        ASSERT_OPTIONAL(Preferences::parseValuePath(path1), path_triple1);
        ASSERT_OPTIONAL(Preferences::parseValuePath(path2), path_triple2);

        ASSERT_NULLOPT(Preferences::parseValuePath(path3));
        ASSERT_NULLOPT(Preferences::parseValuePath(path4));
    }

    /**
     * @brief Validates that a simple config file can be loaded by JSON
     */
    TEST_F(PreferencesTests, InitTest) {
        auto simple_conf_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.conf";

        Preferences::getInstance(false).setConfigLocation(simple_conf_path);

        ASSERT_TRUE(Preferences::getInstance().isInitialized());
    }

    /**
     * @brief Validates that default values can be used in place of loading a file
     */
    TEST_F(PreferencesTests, DefaultValueTests) {
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Reset the held values to be the default
        Preferences::getInstance(false).resetToDefaults();

        auto result = Preferences::getInstance().getPreferenceValue<bool>("SimpleSection.SimpleGroup.val1");

        ASSERT_OPTIONAL(result, false);
    }

    TEST_F(PreferencesTests, ValidateSetConfigValueTest) {
        // First provide the default values
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Reset the held values to be the default
        Preferences::getInstance(false).resetToDefaults();

        // Change the value to 35L
        ASSERT_TRUE(Preferences::getInstance().setPreferenceValue("SimpleSection.SimpleGroup.val2", 35L));

        // Now verify that the value is what we expect it to be
        auto result = Preferences::getInstance().getPreferenceValue<int64_t>("SimpleSection.SimpleGroup.val2");
        ASSERT_OPTIONAL(result, 35L);
    }

    TEST_F(PreferencesTests, ValidateSetUnexpectedTypeFailsTest) {
        // First provide the default values
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Reset the held values to be the default
        Preferences::getInstance(false).resetToDefaults();

        // Try to change the value to 'not an integer'
        ASSERT_FALSE(Preferences::getInstance().setPreferenceValue("SimpleSection.SimpleGroup.val2", "not an integer"));

        // Now verify that the value is what we expect it to be
        auto result = Preferences::getInstance().getPreferenceValue<int64_t>("SimpleSection.SimpleGroup.val2");
        ASSERT_OPTIONAL(result, 5L);
    }

    TEST_F(PreferencesTests, LoadAndValidateSimpleConfigTest) {
        // First provide the default values
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Set the config path now
        auto simple_conf_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.conf";
        Preferences::getInstance(false).setConfigLocation(simple_conf_path);

        // Initialize and validate
        ASSERT_TRUE(Preferences::getInstance().validateLoadedPreferenceTypes());
    }

    TEST_F(PreferencesTests, ValidateEmptyConfigHasDefaultsTest) {
        // First provide the default values
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Next reset the preferences so that the defaults are held
        Preferences::getInstance(false).resetToDefaults();

        // Set the config path now
        auto simple_conf_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "empty.conf";
        Preferences::getInstance(false).setConfigLocation(simple_conf_path);

        // Initialize and validate
        ASSERT_TRUE(Preferences::getInstance().validateLoadedPreferenceTypes());

        // Next compare the two config mappings to verify that they are equal
        ASSERT_TRUE(std::equal(Preferences::getInstance().getSections().begin(),
                               Preferences::getInstance().getSections().end(),
                               simple_conf_defaults.begin(),
                               [](const auto& a, const auto& b) {
                                   return a.first == b.first &&
                                          compareSections(a.second, b.second);
                               }));
    }

    TEST_F(PreferencesTests, ValidateLoadedConfigValuesAreExpected) {
        // First provide the default values
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Do not reset to defaults here as we want to make sure that all values
        //   loaded are definitely taken from the file, and are not defaults

        // Set the config path now
        auto simple_conf_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple_exact.conf";
        Preferences::getInstance(false).setConfigLocation(simple_conf_path);

        // Initialize and validate
        ASSERT_TRUE(Preferences::getInstance().validateLoadedPreferenceTypes());

        const auto& loaded_sections = Preferences::getInstance().getSections();

        TEST_COUT << "Checking all configs." << std::endl;
        for(auto&& [sec_name, section] : simple_conf_defaults) {
            // First, validate that this section exists
            ASSERT_NE(loaded_sections.count(sec_name), 0);

            const auto& loaded_section = loaded_sections.at(sec_name);

            for(auto&& [group_name, group] : section.groups) {
                // Validate that this group exists
                ASSERT_NE(loaded_section.groups.count(group_name), 0);

                const auto& loaded_group = loaded_section.groups.at(group_name);

                for(auto&& [config_name, config] : group.configs) {
                    // Validate that this config exists
                    ASSERT_NE(loaded_group.configs.count(config_name), 0);

                    ASSERT_EQ(loaded_group.configs.at(config_name), config);
                }
            }
        }
    }

    TEST_F(PreferencesTests, ValidateWritePreferencesToFileTest) {
        // First provide the default values
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Next reset the preferences so that the defaults are held
        Preferences::getInstance(false).resetToDefaults();

        // Make sure that the directory we are going to write to exists
        auto base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
        if(!std::filesystem::exists(base_path)) {
            TEST_COUT << "Directory " << base_path
                      << " does not exist, creating." << std::endl;
            ASSERT_TRUE(std::filesystem::create_directory(base_path));
        }

        // Set the config path now
        auto conf_out_path = base_path / "sample_out.conf";
        Preferences::getInstance(false).setConfigLocation(conf_out_path);

        ASSERT_TRUE(Preferences::getInstance().writeToFile());

        ASSERT_TRUE(std::filesystem::exists(conf_out_path));

        // Reload the config into memory now
        ASSERT_TRUE(Preferences::getInstance().validateLoadedPreferenceTypes());

        // Next compare the two config mappings to verify that they are equal
        ASSERT_TRUE(std::equal(Preferences::getInstance().getSections().begin(),
                               Preferences::getInstance().getSections().end(),
                               simple_conf_defaults.begin(),
                               [](const auto& a, const auto& b) {
                                   return a.first == b.first &&
                                          compareSections(a.second, b.second);
                               }));
    }

    TEST_F(PreferencesTests, ValidateLoadConfigWithDefaultValue) {
        // First provide the default values
        Preferences::getInstance(false).setDefaultValues(simple_conf_defaults);

        // Do not load default values into memory

        // Set the config path now
        auto simple_conf_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple_with_default.conf";
        Preferences::getInstance(false).setConfigLocation(simple_conf_path);

        // Load the config into memory now
        ASSERT_TRUE(Preferences::getInstance().validateLoadedPreferenceTypes());

        // Next compare the two config mappings to verify that they are equal
        auto result = Preferences::getInstance().getPreferenceValue<int64_t>("SimpleSection.SimpleGroup.val2");
        ASSERT_OPTIONAL(result, 5L);
    }
}

