#ifndef PREPROCESSOR_UTILS_H
# define PREPROCESSOR_UTILS_H

# define STR(X) #X
# define PRIM_CAT(X, Y) X ## Y
# define CONCAT(X, Y) PRIM_CAT(X, Y)
# define EXPAND(...) __VA_ARGS__

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

#endif

