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

# define ASSERT_STATUS(VAL, EXPECTED) \
    ASSERT_EQ(VAL.error(), EXPECTED)
# define ASSERT_SUCCEEDED(VAL)    \
    ASSERT_TRUE(IS_SUCCESS(VAL)); \

# define TEST_COUT std::cerr << "[          ] [ INFO ]"
# define TEST_CERR std::cerr << "[          ] [ ERR  ]"

namespace HMDT::UnitTests {
    /**
     * @brief Null output buffer
     * @details From https://stackoverflow.com/a/11826666
     */
    class NullBuffer: public std::streambuf {
        public:
            int overflow(int c) { return c; }
    };

    /**
     * @brief Null output stream
     * @details From https://stackoverflow.com/a/11826666
     */
    class NullStream: public std::ostream {
        public:
            NullStream(): std::ostream(&m_sb) { }
        private:
            NullBuffer m_sb;
    };

    static NullStream cnul;

    bool useVerboseOutput();

    std::filesystem::path getTestProgramPath();

    void registerTestLogOutputFunction(bool, bool, bool, bool);
}

#endif

