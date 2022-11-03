/**
 * @file Monad.h
 *
 * @brief Defines a Monadic optional type, which acts as a light wrapper around
 *        std::optional, just with added support for monadic functions such as
 *        transform, andThen, and orElse.
 *
 * @details Based on proposal p0798r3, found here:
 *          http://open-std.org/JTC1/SC22/WG21/docs/papers/2019/p0798r3.html
 */

#ifndef MONAD_H
# define MONAD_H

# include <optional>
# include <functional>
# include <variant> // std::monostate

namespace HMDT {
    template<typename T>
    struct IsOptional: public std::false_type{};

    template<typename T>
    struct IsOptional<std::optional<T>>: public std::true_type{};

    template<typename T>
    constexpr bool IsOptional_v = IsOptional<T>::value;

    template<typename T>
    struct AsMonostateIfVoid { using Type = T; };

    template<>
    struct AsMonostateIfVoid<void> { using Type = std::monostate; };

    template<typename T>
    using AsMonostateIfVoid_t = typename AsMonostateIfVoid<T>::Type;

    /**
     * @brief A light-weight wrapper around a std::optional, with support for
     *        monadic functions/operations
     *
     * @tparam T The type to be stored in the optional
     */
    template<typename T>
    class MonadOptional {
        public:
            using OptionalType = std::optional<AsMonostateIfVoid_t<T>>;
            using value_type = typename OptionalType::value_type;

            MonadOptional() noexcept: m_opt() { }

            /**
             * @brief Constructs a MonadOptional from an existing std::optional.
             */
            MonadOptional(const OptionalType& opt): m_opt(opt)
            { }
            MonadOptional(std::nullopt_t) noexcept: m_opt(std::nullopt)
            { }

            template<typename... Args>
            constexpr explicit MonadOptional(std::in_place_t i, Args&&... args):
                m_opt(i, args...)
            { }
            template<typename U, typename... Args>
            constexpr explicit MonadOptional(std::in_place_t i,
                                             std::initializer_list<U> ilist,
                                             Args&&... args):
                m_opt(i, ilist, args...)
            { }

            template<typename U = value_type,
                     typename = std::enable_if_t<!std::is_base_of_v<MonadOptional<value_type>, U> &&
                                                 !IsOptional_v<U>>
                    >
            constexpr MonadOptional(U&& value): m_opt(std::forward<U>(value))
            { }

            MonadOptional(const MonadOptional<value_type>& mopt): m_opt(mopt.m_opt)
            { }
            MonadOptional(MonadOptional<value_type>&& mopt): m_opt(std::move(mopt.m_opt))
            { }

            MonadOptional& operator=(const MonadOptional& mopt) {
                m_opt = mopt.m_opt;

                return *this;
            }

            ////////////////////////////////////////////////////////////////////

            constexpr const value_type* operator->() const {
                return m_opt.operator->();
            }
            constexpr value_type* operator->() {
                return m_opt.operator->();
            }
            constexpr const value_type& operator*() const& {
                return m_opt.operator*();
            }
            constexpr value_type& operator*() & {
                return m_opt.operator*();
            }
            constexpr const value_type&& operator*() const&& {
                return std::forward<value_type>(m_opt.operator*());
            }
            constexpr value_type&& operator*() && {
                return std::forward<value_type>(m_opt.operator*());
            }

            constexpr explicit operator bool() const noexcept {
                return (bool)m_opt;
            }
            constexpr bool has_value() const noexcept {
                return m_opt.has_value();
            }


            constexpr value_type& value() & {
                return m_opt.value();
            }
            constexpr const value_type& value() const& {
                return m_opt.value();
            }
            constexpr value_type&& value() && {
                return m_opt.value();
            }
            constexpr const value_type&& value() const&& {
                return m_opt.value();
            }

            template<typename U>
            constexpr value_type value_or(U&& default_value) const& {
                return m_opt.template value_or<U>(std::forward<U>(default_value));
            }
            template<typename U>
            constexpr value_type value_or(U&& default_value) && {
                return m_opt.template value_or<U>(std::forward<U>(default_value));
            }

            void swap(MonadOptional& mopt) noexcept(std::is_nothrow_move_constructible_v<value_type> &&
                                                    std::is_nothrow_swappable_v<value_type>)
            {
                return m_opt.swap(mopt.m_opt);
            }

            void reset() noexcept {
                m_opt.reset();
            }

            template<typename... Args>
            value_type& emplace(Args&&... args) {
                return m_opt.template emplace<Args...>(std::forward<Args>(args)...);
            }
            template<typename U, typename... Args>
            value_type& emplace(std::initializer_list<U> ilist, Args&&... args) {
                return m_opt.template emplace<U, Args...>(ilist, std::forward<Args>(args)...);
            }

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Transforms value_type into R with the given function, if this
             *        object holds a value.
             *
             * @tparam R The type to transform into
             * @param func The transformation function
             *
             * @return A MonadOptional containing either the return value of
             *         func, or std::nullopt if this object does not hold a
             *         value.
             */
            template<typename R>
            MonadOptional<R> transform(std::function<R(const value_type&)> func) {
                if(m_opt) {
                    return MonadOptional<R>{std::optional<R>{func(*m_opt)}};
                }

                return std::nullopt;
            }

            /**
             * @brief Performs an additional operation if this function holds
             *        a value, and returns the result.
             *
             * @tparam R The type returned by the given function
             * @param func A function which returns a MonadOptional
             *
             * @return The return value of func, or std::nullopt if this object
             *         does not hold a value.
             */
            template<typename R>
            MonadOptional<R> andThen(std::function<MonadOptional<R>(value_type&)> func) {
                if(m_opt) {
                    return func(*m_opt);
                }

                return std::nullopt;
            }

            /**
             * @brief Performs an additional operation if this function holds
             *        a value, and returns a std::monostate.
             *
             * @param func A function which returns nothing.
             *
             * @return A std::monostate, or std::nullopt if this object does not
             *         hold a value.
             */
            MonadOptional<std::monostate> andThen(std::function<void(value_type&)> func)
            {
                if(m_opt) {
                    func(*m_opt);
                    return std::monostate{};
                }

                return std::nullopt;
            }

            /**
             * @brief Performs an additional operation if this function holds
             *        a value, and returns the result.
             *
             * @tparam R The type returned by the given function
             * @param func A function which returns a MonadOptional
             *
             * @return The return value of func, or std::nullopt if this object
             *         does not hold a value.
             */
            template<typename R>
            MonadOptional<R> andThen(std::function<MonadOptional<R>(const value_type&)> func) const
            {
                if(m_opt) {
                    return func(*m_opt);
                }

                return std::nullopt;
            }

            /**
             * @brief Performs an additional operation if this function holds
             *        a value, and returns a std::monostate.
             *
             * @param func A function which returns nothing.
             *
             * @return A std::monostate, or std::nullopt if this object does not
             *         hold a value.
             */
            MonadOptional<std::monostate> andThen(std::function<void(const value_type&)> func) const
            {
                if(m_opt) {
                    func(*m_opt);
                    return std::monostate{};
                }

                return std::nullopt;
            }

            /**
             * @brief Returns this optional if a value is held, otherwise it
             *        calls the given function.
             *
             * @tparam R The type returned by the given function. Must be
             *           convertible to a value_type.
             * @param func A function to call if this object does not hold a
             *             value
             *
             * @return If no value is held, then either the return value of func
             *         will be returned, or std::nullopt if R is void
             */
            template<typename R,
                     typename = std::enable_if_t<std::is_convertible_v<R, value_type> ||
                                                 std::is_void_v<R>>>
            MonadOptional<value_type> orElse(std::function<R()> func) {
                if(m_opt) {
                    return m_opt;
                }

                if constexpr(std::is_same_v<R, void>) {
                    return std::nullopt;
                } else {
                    return func();
                }
            }

            /**
             * @brief Returns this optional if a value is held, otherwise it
             *        returns the provided default value
             *
             * @tparam R The type of the default value. Must be convertible to a
             *           value_type.
             * @param value The default value to return if this object does not
             *              hold a value.
             *
             * @return If no value is held, then the given default value is
             *         returned, otherwise this optional's value is returned.
             */
            template<typename R,
                     typename = std::enable_if_t<std::is_convertible_v<R, value_type> ||
                                                 std::is_constructible_v<value_type, R>>>
            value_type orElse(const R& value) {
                if(m_opt) {
                    return *m_opt;
                }

                return value_type{value};
            }

            /**
             * @brief Gets the wrapped std::optional
             */
            OptionalType& getWrapped() {
                return m_opt;
            }

            /**
             * @brief Gets the wrapped std::optional
             */
            const OptionalType& getWrapped() const {
                return m_opt;
            }

        private:
            OptionalType m_opt;
    };

    // Compare with MonadOptional
    template<typename T, typename U>
    constexpr bool operator==(const MonadOptional<T>& lhs,
                              const MonadOptional<U>& rhs)
    {
        return lhs.getWrapped() == rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator!=(const MonadOptional<T>& lhs,
                              const MonadOptional<U>& rhs)
    {
        return lhs.getWrapped() != rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator<(const MonadOptional<T>& lhs,
                              const MonadOptional<U>& rhs)
    {
        return lhs.getWrapped() < rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator<=(const MonadOptional<T>& lhs,
                              const MonadOptional<U>& rhs)
    {
        return lhs.getWrapped() <= rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator>(const MonadOptional<T>& lhs,
                              const MonadOptional<U>& rhs)
    {
        return lhs.getWrapped() > rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator>=(const MonadOptional<T>& lhs,
                              const MonadOptional<U>& rhs)
    {
        return lhs.getWrapped() >= rhs.getWrapped();
    }

    // Compare with nullopt
    template<typename T>
    constexpr bool operator==(const MonadOptional<T>& lhs, std::nullopt_t rhs) {
        return lhs.getWrapped() == rhs;
    }
    template<typename T>
    constexpr bool operator!=(const MonadOptional<T>& lhs, std::nullopt_t rhs) {
        return lhs.getWrapped() != rhs;
    }
    template<typename T>
    constexpr bool operator<(const MonadOptional<T>& lhs, std::nullopt_t rhs) {
        return lhs.getWrapped() < rhs;
    }
    template<typename T>
    constexpr bool operator<=(const MonadOptional<T>& lhs, std::nullopt_t rhs) {
        return lhs.getWrapped() <= rhs;
    }
    template<typename T>
    constexpr bool operator>(const MonadOptional<T>& lhs, std::nullopt_t rhs) {
        return lhs.getWrapped() > rhs;
    }
    template<typename T>
    constexpr bool operator>=(const MonadOptional<T>& lhs, std::nullopt_t rhs) {
        return lhs.getWrapped() >= rhs;
    }

    template<typename T>
    constexpr bool operator==(std::nullopt_t lhs, const MonadOptional<T>& rhs) {
        return lhs == rhs.getWrapped();
    }
    template<typename T>
    constexpr bool operator!=(std::nullopt_t lhs, const MonadOptional<T>& rhs) {
        return lhs != rhs.getWrapped();
    }
    template<typename T>
    constexpr bool operator<(std::nullopt_t lhs, const MonadOptional<T>& rhs) {
        return lhs < rhs.getWrapped();
    }
    template<typename T>
    constexpr bool operator<=(std::nullopt_t lhs, const MonadOptional<T>& rhs) {
        return lhs <= rhs.getWrapped();
    }
    template<typename T>
    constexpr bool operator>(std::nullopt_t lhs, const MonadOptional<T>& rhs) {
        return lhs > rhs.getWrapped();
    }
    template<typename T>
    constexpr bool operator>=(std::nullopt_t lhs, const MonadOptional<T>& rhs) {
        return lhs >= rhs.getWrapped();
    }

    // Compare with value
    template<typename T, typename U>
    constexpr bool operator==(const MonadOptional<T>& lhs, const U& rhs) {
        return lhs.getWrapped() == rhs;
    }
    template<typename T, typename U>
    constexpr bool operator!=(const MonadOptional<T>& lhs, const U& rhs) {
        return lhs.getWrapped() != rhs;
    }
    template<typename T, typename U>
    constexpr bool operator<(const MonadOptional<T>& lhs, const U& rhs) {
        return lhs.getWrapped() < rhs;
    }
    template<typename T, typename U>
    constexpr bool operator<=(const MonadOptional<T>& lhs, const U& rhs) {
        return lhs.getWrapped() <= rhs;
    }
    template<typename T, typename U>
    constexpr bool operator>(const MonadOptional<T>& lhs, const U& rhs) {
        return lhs.getWrapped() > rhs;
    }
    template<typename T, typename U>
    constexpr bool operator>=(const MonadOptional<T>& lhs, const U& rhs) {
        return lhs.getWrapped() >= rhs;
    }

    template<typename T, typename U>
    constexpr bool operator==(const U& lhs, const MonadOptional<T>& rhs) {
        return lhs == rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator!=(const U& lhs, const MonadOptional<T>& rhs) {
        return lhs != rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator<(const U& lhs, const MonadOptional<T>& rhs) {
        return lhs < rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator<=(const U& lhs, const MonadOptional<T>& rhs) {
        return lhs <= rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator>(const U& lhs, const MonadOptional<T>& rhs) {
        return lhs > rhs.getWrapped();
    }
    template<typename T, typename U>
    constexpr bool operator>=(const U& lhs, const MonadOptional<T>& rhs) {
        return lhs >= rhs.getWrapped();
    }

    /**
     * @brief Helper type wrapping a MonadOptional<std::reference_wrapper<T>>
     *
     * @tparam T The type to be stored in the optional
     */
    template<typename T>
    using MonadOptionalRef = MonadOptional<std::reference_wrapper<T>>;
}

#endif

