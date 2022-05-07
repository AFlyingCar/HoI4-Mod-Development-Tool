
#include "gtest/gtest.h"

#include "Version.h"

TEST(VersionTests, TestEquality) {
    HMDT::Version v1("1.0.3.4");
    HMDT::Version v2("1.0.3.5");

    ASSERT_EQ(v1, v1);
    ASSERT_NE(v1, v2);
}

TEST(VersionTests, TestGreaterThanLessThan) {
    HMDT::Version v1("1.0.3.4");
    HMDT::Version v2("1.0.3.5");

    ASSERT_GT(v2, v1);
    ASSERT_LT(v1, v2);
}

