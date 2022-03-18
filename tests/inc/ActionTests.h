
#ifndef ACTION_TESTS_H
# define ACTION_TESTS_H

# include "gtest/gtest.h"

namespace MapNormalizer::UnitTests {
    class ActionTests: public ::testing::Test {
        protected:
            void SetUp() override;
            void TearDown() override;
    };
}

#endif

