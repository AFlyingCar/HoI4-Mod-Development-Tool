#ifndef PREPROCESSOR_UTILS_H
# define PREPROCESSOR_UTILS_H

# define STR(X) #X
# define PRIM_CAT(X, Y) X ## Y
# define CONCAT(X, Y) PRIM_CAT(X, Y)
# define EXPAND(...) __VA_ARGS__
# define HMDT_LOCALIZE(...) __VA_ARGS__

# define HMDT_UNIQUE_NAME(PREFIX) CONCAT(CONCAT(CONCAT(PREFIX, _L),__LINE__), \
                                  CONCAT(_C,__COUNTER__))

/**
 * @brief Begins a block of code to run during static initialization
 */
# define ON_STATIC_INIT_BEGIN_BLOCK \
    ON_STATIC_INIT_BEGIN_BLOCK_IMPL(EXPAND(HMDT_UNIQUE_NAME(___ON_STATIC_INIT___)))

# define ON_STATIC_INIT_BEGIN_BLOCK_IMPL(OBJ_NAME)  \
    struct OBJ_NAME {                          \
        OBJ_NAME()

# define ON_STATIC_INIT_END_BLOCK } HMDT_UNIQUE_NAME(___ON_STATIC_INIT_INSTANCE___)

# ifdef __has_include
#  if __has_include(<version>)
#   include <version>
#  endif
# endif

// Define feature-test macros if they aren't already defined
// If __has_cpp_attribute is not defined though, then we must assume that the
//   feature in question does not exist (this should only be used for features
//   newer than C++20)
# ifdef __has_cpp_attribute
#  define HAS_ATTRIBUTE(attr) __has_cpp_attribute(attr)
# else
#  define HAS_ATTRIBUTE(attr) 0
# endif

#endif

