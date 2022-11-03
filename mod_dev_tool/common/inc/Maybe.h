/**
 * @file Maybe.h
 *
 * @brief Defines a custom Monadic type, which can hold an error code. Also
 *        defines several helper macros for quickly logging and
 *        returning/forwarding error codes.
 *
 * @details This class is specifically meant for use with the macros defined at
 *          the bottom for logging and returning/forwarding various error codes.
 *
 * @code{.cpp}
 *     // Notice that the function must return an HMDT::Maybe<...>
 *     // If you wish to return an error code without a value, you can return
 *     //   either a Maybe<std::monostate> or the helper MaybeVoid type.
 *     HMDT::Maybe<int> loadData(const std::filesystem::path& file_path) {
 *         // Return an error code if the file does not exist, otherwise we'll
 *         //   continue on
 *         RETURN_ERROR_IF(!std::filesystem::exists(file_path), HMDT::ErrorCodes::FILE_NOT_FOUND);
 *
 *         // Load some data...
 *         //       ...
 *
 *         // Assume we got some path leaf
 *         auto path2 = file_path / leaf;
 *
 *         // Defer to some more specific load function
 *         auto result = loadSpecific(path2);
 *
 *         // If the above function failed, this will log the current file+line+function and pass the error up the chain
 *         RETURN_IF_ERROR(result);
 *
 *         // We'll just transform whatever the result was, this is just a toy
 *         //   example after all
 *         return result.transform<int>([](const auto& v) {
 *             return v * 5;
 *         });
 *     }
 * @endcode
 *
 * Not mentioned in the above example is the RETURN_ERROR() macro, which simply
 *      logs and returns an error with no extra checks. This is mostly used by
 *      the other macros to reduce code copy+pasting, but could be useful as 
 *      well if you simply want to just return an error code and nothing else.
 */

#ifndef MAYBE_H
# define MAYBE_H

# include <system_error> // std::error_code

# include "Monad.h"
# include "TypeTraits.h"

# include "Logger.h"

namespace HMDT {
    /**
     * @brief A special version of MonadOptional specifically for use with
     *        error checking functions.
     *
     * @tparam T The type to be stored in the optional
     */
    template<typename T>
    class Maybe: public MonadOptional<T> {
        public:
            // Make sure we don't hide the base class overloads
            using MonadOptional<T>::orElse;
            using MonadOptional<T>::getWrapped;

            Maybe() noexcept: MonadOptional<T>(), m_ec() { }

            /**
             * @brief Constructs a Maybe from an existing std::optional.
             */
            Maybe(const typename MonadOptional<T>::OptionalType& opt):
                MonadOptional<T>(opt), m_ec()
            { }

            template<typename... Args>
            constexpr explicit Maybe(std::in_place_t i, Args&&... args):
                MonadOptional<T>(i, args...), m_ec()
            { }
            template<typename U, typename... Args>
            constexpr explicit Maybe(std::in_place_t i,
                                     std::initializer_list<U> ilist,
                                     Args&&... args):
                MonadOptional<T>(i, ilist, args...), m_ec()
            { }

            template<typename U = T>
            Maybe(const U& value):
                MonadOptional<T>(value), m_ec()
            { }

            template<typename U = T,
                     typename = std::enable_if_t<!std::is_same_v<U, Maybe<T>> &&
                                                 !std::is_same_v<U, MonadOptional<T>> &&
                                                 std::is_convertible_v<U, typename MonadOptional<T>::value_type>>
                    >
            constexpr Maybe(U&& value): MonadOptional<T>(std::forward<U>(value))
            { }

            template<typename U = T>
            Maybe(const Maybe<U>& maybe):
                Maybe<T>(maybe, Identity_t<U>{})
            { }

            template<typename U = T>
            Maybe(Maybe<U>&& maybe):
                Maybe<T>(std::move(maybe), Identity_t<U>{})
            { }

            Maybe(std::error_code ec):
                MonadOptional<T>(std::nullopt), m_ec(ec)
            { }

            Maybe(Maybe<T>&& maybe):
                MonadOptional<T>(std::forward<MonadOptional<T>>(maybe)),
                m_ec(std::move(maybe.m_ec))
            { }

            Maybe& operator=(const Maybe& maybe) {
                MonadOptional<T>::operator=(maybe);

                m_ec = maybe.m_ec;

                return *this;
            }

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Performs an additional operation if this function holds
             *        a value, and returns a std::monostate.
             *
             * @param func A function which returns nothing.
             *
             * @return A std::monostate, or std::nullopt if this object does not
             *         hold a value.
             */
            MonadOptional<std::monostate> andThen(std::function<void(typename MonadOptional<T>::value_type&)> func)
            {
                if(getWrapped()) {
                    func(*getWrapped());
                    return std::monostate{};
                }

                return std::nullopt;
            }

            /**
             * @brief Performs an additional operation if this function holds
             *        a value, and returns the result.
             *
             * @tparam R The type returned by the given function
             * @param func A function which returns a Maybe
             *
             * @return The return value of func, or std::nullopt if this object
             *         does not hold a value.
             */
            template<typename R>
            Maybe<R> andThen(std::function<Maybe<R>(typename MonadOptional<T>::value_type&)> func) {
                if(getWrapped()) {
                    return func(*getWrapped());
                }

                return std::nullopt;
            }

            /**
             * @brief Performs an additional operation if this function holds
             *        a value, and returns the result.
             *
             * @tparam R The type returned by the given function
             * @param func A function which returns a Maybe
             *
             * @return The return value of func, or std::nullopt if this object
             *         does not hold a value.
             */
            template<typename R>
            Maybe<R> andThen(std::function<Maybe<R>(const typename MonadOptional<T>::value_type&)> func) const {
                if(getWrapped()) {
                    return func(*getWrapped());
                }

                return std::nullopt;
            }

            /**
             * @brief Returns this optional if a value is held, otherwise it
             *        calls the given function.
             *
             * @tparam R The type returned by the given function. Must be
             *           convertible to a T.
             * @param func A function to call if this object does not hold a
             *             value
             *
             * @return If no value is held, then either the return value of func
             *         will be returned, or std::nullopt if R is void
             */
            template<typename R,
                     typename = std::enable_if_t<std::is_convertible_v<R, typename MonadOptional<T>::value_type> ||
                                                 std::is_void_v<R>>>
            Maybe<T> orElse(std::function<Maybe<R>()> func) {
                if(getWrapped()) {
                    return getWrapped();
                }

                if constexpr(std::is_same_v<R, void>) {
                    return std::nullopt;
                } else {
                    return func();
                }
            }

            ////////////////////////////////////////////////////////////////////

            /**
             * @brief Gets the error code held by this Maybe
             *
             * @return The error code held by this Maybe.
             */
            const std::error_code& error() const {
                return m_ec;
            }

        private:
            // Note that we have to do this private constructor thing rather
            //   than just specialization due to a GCC bug that prevents
            //   specialization in non-namespace scopes

            template<typename U = T,
                     typename = std::enable_if_t<std::is_constructible_v<typename MonadOptional<T>::value_type, U>>>
            Maybe(const Maybe<U>& maybe, Identity_t<U>):
                MonadOptional<T>(maybe.getWrapped()),
                m_ec(maybe.error())
            { }

            Maybe(const Maybe<std::monostate>& maybe, Identity_t<std::monostate>):
                MonadOptional<T>(std::nullopt), m_ec(maybe.error())
            { }

            template<typename U = T,
                     typename = std::enable_if_t<std::is_constructible_v<typename MonadOptional<T>::value_type, U&&>>>
            Maybe(Maybe<U>&& maybe, Identity_t<U>):
                MonadOptional<T>(std::move(maybe)),
                m_ec(std::move(maybe.error()))
            { }

            Maybe(Maybe<std::monostate>&& maybe, Identity_t<std::monostate>):
                MonadOptional<T>(std::nullopt), m_ec(std::move(maybe.error()))
            { }

            //! The std::error_code held by this Maybe
            std::error_code m_ec;
    };

    // Compare with Maybe
    template<typename T, typename U>
    constexpr bool operator==(const Maybe<T>& lhs,
                              const Maybe<U>& rhs)
    {
        return lhs.getWrapped() == rhs.getWrapped() &&
               lhs.error() == rhs.error();
    }
    template<typename T, typename U>
    constexpr bool operator!=(const Maybe<T>& lhs,
                              const Maybe<U>& rhs)
    {
        return lhs.getWrapped() != rhs.getWrapped() &&
               lhs.error() != rhs.error();
    }
    template<typename T, typename U>
    constexpr bool operator<(const Maybe<T>& lhs,
                              const Maybe<U>& rhs)
    {
        return lhs.getWrapped() < rhs.getWrapped() &&
               lhs.error() < rhs.error();
    }
    template<typename T, typename U>
    constexpr bool operator<=(const Maybe<T>& lhs,
                              const Maybe<U>& rhs)
    {
        return lhs.getWrapped() <= rhs.getWrapped() &&
               lhs.error() <= rhs.error();
    }
    template<typename T, typename U>
    constexpr bool operator>(const Maybe<T>& lhs,
                              const Maybe<U>& rhs)
    {
        return lhs.getWrapped() > rhs.getWrapped() &&
               lhs.error() > rhs.error();
    }
    template<typename T, typename U>
    constexpr bool operator>=(const Maybe<T>& lhs,
                              const Maybe<U>& rhs)
    {
        return lhs.getWrapped() >= rhs.getWrapped() &&
               lhs.error() >= rhs.error();
    }

    // Compare with std::error_code
    template<typename T>
    constexpr bool operator==(const Maybe<T>& lhs, const std::error_code& rhs) {
        return lhs.error() == rhs;
    }
    template<typename T>
    constexpr bool operator!=(const Maybe<T>& lhs, const std::error_code& rhs) {
        return lhs.error() != rhs;
    }
    template<typename T>
    constexpr bool operator<(const Maybe<T>& lhs, const std::error_code& rhs) {
        return lhs.error() < rhs;
    }
    template<typename T>
    constexpr bool operator<=(const Maybe<T>& lhs, const std::error_code& rhs) {
        return lhs.error() <= rhs;
    }
    template<typename T>
    constexpr bool operator>(const Maybe<T>& lhs, const std::error_code& rhs) {
        return lhs.error() > rhs;
    }
    template<typename T>
    constexpr bool operator>=(const Maybe<T>& lhs, const std::error_code& rhs) {
        return lhs.error() >= rhs;
    }

    template<typename T>
    constexpr bool operator==(const std::error_code& lhs, const Maybe<T>& rhs) {
        return lhs == rhs.error();
    }
    template<typename T>
    constexpr bool operator!=(const std::error_code& lhs, const Maybe<T>& rhs) {
        return lhs != rhs.error();
    }
    template<typename T>
    constexpr bool operator<(const std::error_code& lhs, const Maybe<T>& rhs) {
        return lhs < rhs.error();
    }
    template<typename T>
    constexpr bool operator<=(const std::error_code& lhs, const Maybe<T>& rhs) {
        return lhs <= rhs.error();
    }
    template<typename T>
    constexpr bool operator>(const std::error_code& lhs, const Maybe<T>& rhs) {
        return lhs > rhs.error();
    }
    template<typename T>
    constexpr bool operator>=(const std::error_code& lhs, const Maybe<T>& rhs) {
        return lhs >= rhs.error();
    }

    /**
     * @brief Helper type wrapping a Maybe<std::reference_wrapper<T>>
     *
     * @tparam T The type to be stored in the optional
     */
    template<typename T>
    using MaybeRef = Maybe<std::reference_wrapper<T>>;

    /**
     * @brief Utility Maybe for void functions
     */
    using MaybeVoid = Maybe<std::monostate>;

    template<typename T>
    Maybe<T> asMaybe(const MonadOptional<T>& opt) {
        return Maybe<T>(opt);
    }

    template<typename T>
    Maybe<T> asMaybe(MonadOptional<T>&& opt) {
        return Maybe<T>(std::move(opt));
    }
}

/**
 * @brief Evaluates to true if the MAYBE has a value or if the stored error_code
 *        is 0
 */
# define IS_SUCCESS(MAYBE) \
    [](auto&& maybe) { return ( maybe.has_value() || maybe.error().value() == 0 ); }(MAYBE)

/**
 * @brief Negation of IS_SUCCESS()
 */
# define IS_FAILURE(MAYBE) \
    ( ! IS_SUCCESS(MAYBE) )

/**
 * @brief Logs an error code to ERROR
 * @details Writes "Returning error - [{CATEGORY_NAME}] 0x{CODE} '{MESSAGE}'
 *
 * @param ERROR_CODE The error code to log
 */
# define WRITE_ERROR_CODE(ERROR_CODE) \
    WRITE_ERROR("Returning error - [", ( ERROR_CODE ).category().name(), "] 0x", FHEX(( ERROR_CODE ).value()), " '", ( ERROR_CODE ).message(), "'")

/**
 * @brief Logs an error code to ERROR if the MAYBE contains an error
 *
 * @param MAYBE The Maybe to check
 */
# define WRITE_IF_ERROR(MAYBE)                  \
    do {                                        \
        if(IS_FAILURE(MAYBE)) {                 \
            WRITE_ERROR_CODE( MAYBE .error() ); \
        }                                       \
    } while(0)

/**
 * @brief Returns an ERROR_CODE from the current function.
 *
 * @param ERROR_CODE The error code to return
 *
 * @return A HMDT::Maybe<std::monostate> containing ERROR_CODE
 */
# define RETURN_ERROR(ERROR_CODE)                                      \
    do {                                                               \
        WRITE_ERROR_CODE( ERROR_CODE );                                \
        return HMDT::Maybe<std::monostate>( ERROR_CODE );              \
    } while(0)

/**
 * @brief Returns the given MAYBE if the MAYBE does not contain a value
 * @details This effectively logs the location where the error was discovered,
 *          and passes it up the stack so that it can be handled up the stack
 *
 * @param MAYBE The Maybe to check
 *
 * @return A HMDT::Maybe<std::monostate> containing MAYBE's error_code if the
 *         given MAYBE does not hold a value.
 */
# define RETURN_IF_ERROR(MAYBE)                                               \
    do {                                                                      \
        if(IS_FAILURE(MAYBE)) {                                               \
            RETURN_ERROR( MAYBE .error() );                                   \
        }                                                                     \
    } while(0)

/**
 * @brief Returns a value if the given MAYBE does not contain a value
 * @details This effectively logs the location where the error was discovered,
 *          and passes it up the stack so that it can be handled up the stack
 *
 * @param MAYBE The Maybe to check
 * @param VALUE The value to return if the check fails
 *
 * @return VALUE if the given MAYBE does not hold a value.
 */
# define RETURN_VALUE_IF_ERROR(MAYBE, VALUE)                                  \
    do {                                                                      \
        if(IS_FAILURE(MAYBE)) {                                               \
            WRITE_ERROR_CODE( MAYBE .error() );                               \
            return VALUE ;                                                    \
        }                                                                     \
    } while(0)

/**
 * @brief Returns ERROR_CODE if COND is true
 *
 * @param COND The condition to check
 * @param ERROR_CODE The error code to return if COND is true
 *
 * @return See RETURN_ERROR
 */
# define RETURN_ERROR_IF(COND, ERROR_CODE)                                 \
    do {                                                                   \
        if ( COND ) {                                                      \
            RETURN_ERROR( ERROR_CODE );                                    \
        }                                                                  \
    } while(0)


#endif

