#ifndef TEST_UTILS_H
# define TEST_UTILS_H

# include <filesystem>

# define STR(X) #X
# define PRIM_CONCAT(A,B) A##B
# define CONCAT(A,B) PRIM_CONCAT(A, B)

# define ASSERT_NULLOPT(VAL) ASSERT_EQ(VAL, std::nullopt)
# define ASSERT_VALID(VAL) ASSERT_NE(VAL, std::nullopt)
# define ASSERT_OPTIONAL(VAL, EXPECTED) \
    ASSERT_VALID(VAL); \
    ASSERT_EQ(*VAL, EXPECTED)
# define ASSERT_OPTIONAL_FLOAT(VAL, EXPECTED) \
    ASSERT_VALID(VAL); \
    ASSERT_FLOAT_EQ(*VAL, EXPECTED)

namespace MapNormalizer::UnitTests {
    bool useVerboseOutput();

    std::filesystem::path getTestProgramPath();
}

#endif

