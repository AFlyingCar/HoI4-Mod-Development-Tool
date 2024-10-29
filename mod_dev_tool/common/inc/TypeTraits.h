#ifndef TYPE_TRAITS_H
# define TYPE_TRAITS_H

# include <type_traits>

namespace HMDT {
    /**
     * @brief Implements the C++20 std::type_identity
     */
    template<typename T>
    struct Identity {
        using type = T;
    };
    template<typename T>
    using Identity_t = typename Identity<T>::type;

    // https://stackoverflow.com/a/39492671
    /**
     * @brief Removes all pointer indirections on the given type.
     */
    template<typename T>
    struct RemoveAllPointers:
        std::conditional_t<std::is_pointer_v<T>,
                           RemoveAllPointers<std::remove_pointer_t<T>>,
                           Identity<T>
                          >
    { };
    template<typename T>
    using RemoveAllPointers_t = typename RemoveAllPointers<T>::type;

/// @cond
    template<typename T>
    T __void_if_no_value(T);
    [[maybe_unused]] void __void_if_no_value();
/// @endcond

    /**
     * @brief Helper type for static_assert which always evaluates to false.
     */
    template<typename...>
    constexpr std::false_type alwaysFalse{};

    /**
     * @brief Checks if a given type is iterable with std::begin() and std::end()
     *
     * @tparam T The type to check
     */
    template<typename T, typename = void>
    struct IsIterable: std::false_type {};

    /**
     * @brief Checks if a given type is iterable with std::begin() and std::end()
     *
     * @tparam T The type to check
     */
    template<typename T>
    struct IsIterable<T, std::void_t<decltype(std::begin(std::declval<T&>())),
                                     decltype(std::end(std::declval<T&>()))
                                    >
                     >: std::true_type {};

    /**
     * @brief Checks if a given type is iterable with std::begin() and std::end()
     *
     * @tparam T The type to check
     */
    template<typename T>
    constexpr bool IsIterable_v = IsIterable<T>::value;
}

#endif

