#ifndef TEST_UTILS_H
# define TEST_UTILS_H

# include <filesystem>

# define STR(X) #X
# define PRIM_CONCAT(A,B) A##B
# define CONCAT(A,B) PRIM_CONCAT(A, B)

namespace MapNormalizer::UnitTests {
    bool useVerboseOutput();

    std::filesystem::path getTestProgramPath();
}

#endif

