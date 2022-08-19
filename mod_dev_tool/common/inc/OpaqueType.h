#ifndef OPAQUE_TYPE_H
# define OPAQUE_TYPE_H

# include "PreprocessorUtils.h"
# include "TypeTraits.h"

namespace HMDT {
    /**
     * @brief Interface for all opaque types.
     *
     * @tparam T The type being wrapped
     */
    template<typename T>
    struct IOpaqueType {
        //! An alias for the wrapped type
        using Type = T;

        constexpr IOpaqueType(): raw() {}
        constexpr IOpaqueType(const T& v): raw(v) {};
        constexpr IOpaqueType(T&& v): raw(std::forward<T>(v)) {};

        //! The wrapped value
        T raw;
    };

    /**
     * @brief An output operator for all opaque types
     *
     * @tparam T The type that was wrapped.
     *
     * @param out The ostream to output to
     * @param opaque The opaquely wrapped value
     *
     * @return out
     */
    template<typename T>
    std::ostream& operator<<(std::ostream& out, const IOpaqueType<T>& opaque) {
        return (out << opaque.raw);
    }

    /**
     * @brief Defines a new opaque type alias of ALIASED_TYPE
     * @details Used like a typedef:
     * @code cpp
     *    HMDT_OPAQUE_ALIAS(int) MyIntAlias;
     * @endcode
     *
     * @param ALIASED_TYPE The type to alias
     */
# define HMDT_OPAQUE_ALIAS(ALIASED_TYPE) \
    HMDT_OPAQUE_ALIAS_IMPL(ALIASED_TYPE, \
                           EXPAND(HMDT_UNIQUE_NAME(__HMDT_OPAQUE_TYPE)))

# define HMDT_OPAQUE_ALIAS_IMPL(ALIASED_TYPE, NAME)                   \
    typedef struct NAME: public HMDT::IOpaqueType<ALIASED_TYPE> {     \
        using HMDT::IOpaqueType<ALIASED_TYPE>::IOpaqueType;           \
        operator ALIASED_TYPE &() { return raw; }                     \
        operator const ALIASED_TYPE &() const { return raw; }         \
    }
}

#endif

