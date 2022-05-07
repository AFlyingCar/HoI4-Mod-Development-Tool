#ifndef TEST_UTILS_H
# define TEST_UTILS_H

# include <filesystem>
# include <iostream>
# include <functional>

# include "PreprocessorUtils.h"

# define ASSERT_NULLOPT(VAL) ASSERT_EQ(VAL, std::nullopt)
# define ASSERT_VALID(VAL) ASSERT_NE(VAL, std::nullopt)
# define ASSERT_OPTIONAL(VAL, EXPECTED) \
    ASSERT_VALID(VAL); \
    ASSERT_EQ(*VAL, EXPECTED)
# define ASSERT_OPTIONAL_FLOAT(VAL, EXPECTED) \
    ASSERT_VALID(VAL); \
    ASSERT_FLOAT_EQ(*VAL, EXPECTED)

# define TEST_COUT std::cerr << "[          ] [ INFO ]"
# define TEST_CERR std::cerr << "[          ] [ ERR  ]"

namespace HMDT::UnitTests {
    bool useVerboseOutput();

    std::filesystem::path getTestProgramPath();
}

#endif

