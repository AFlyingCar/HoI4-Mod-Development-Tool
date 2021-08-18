#ifndef TEST_UTILS_H
# define TEST_UTILS_H

# include <filesystem>
# include <iostream>
# include <functional>

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

# define TEST_COUT std::cerr << "[          ] [ INFO ]"
# define TEST_CERR std::cerr << "[          ] [ ERR  ]"

# define SCOPE_UNIQUE_NAME(PREFIX) CONCAT(PREFIX, __COUNTER__)

# define RUN_AT_SCOPE_END(...) MapNormalizer::UnitTests::_RunAtScopeEnd SCOPE_UNIQUE_NAME(___AT_SCOPE_END) ( __VA_ARGS__ )

namespace MapNormalizer::UnitTests {
    /**
     * @brief Runs the given function when the current scope ends
     */
    struct _RunAtScopeEnd {
        _RunAtScopeEnd(const std::function<void()>& f): m_f(f) { }

        ~_RunAtScopeEnd() {
            m_f();
        }

        std::function<void()> m_f;
    };

    bool useVerboseOutput();

    std::filesystem::path getTestProgramPath();
}

#endif

