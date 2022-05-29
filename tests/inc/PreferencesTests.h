
#ifndef PREFERENCES_TESTS_H
# define PREFERENCES_TESTS_H

# include "gtest/gtest.h"

namespace HMDT::UnitTests {
    class PreferencesTests: public ::testing::Test {
        protected:
            static void SetUpTestSuite();
            static void TearDownTestSuite();

            void SetUp() override;
            void TearDown() override;
    };
}

#endif

