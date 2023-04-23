/**
 * @file Uuid.h
 *
 * @brief Defines a standard, cross-platform C++-based UUID structure for usage
 *        in the project.
 *
 * @details Based on/adapted from this SO answer: https://stackoverflow.com/a/1626302
 */

#ifndef HMDT_UUID_H
# define HMDT_UUID_H

# include <string>
# include <optional>

extern "C" {
# ifdef WIN32
#  include <Rpc.h>
# else
#  include <uuid/uuid.h>
# endif
}

# include "Maybe.h"

namespace HMDT {
    class UUID {
        public:
            using SystemUUIDType = 
# ifdef WIN32
                ::UUID
# else
                uuid_t
# endif
                ;

            /**
             * @brief Specifies how to create the UUID. Note that on Windows all
             *        creation options mean the same thing except for EMPTY.
             */
            enum class CreationParams {
                //! Will generate an empty/nil UUID
                EMPTY,
                //! Will generate a UUID using uuid_generate_time/UuidCreate
                TIME,
                //! Will generate a UUID using uuid_generate_random/UuidCreate
                RANDOM,
                //! Will generate a UUID using uuid_generate/UuidCreate
                BEST
            };

            /**
             * @brief The length every string representation of a UUID will have
             *        "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"
             */
            constexpr static std::uint32_t STRING_REPR_LENGTH = 36;

            UUID(CreationParams = CreationParams::BEST);
            UUID(const UUID&);
            UUID(UUID&&);

            UUID& operator=(const UUID&) noexcept;

            ~UUID();

            bool operator!=(const UUID&) const noexcept;
            bool operator==(const UUID&) const noexcept;
            bool operator<(const UUID&) const noexcept;
            bool operator<=(const UUID&) const noexcept;
            bool operator>(const UUID&) const noexcept;
            bool operator>=(const UUID&) const noexcept;

            bool operator==(std::size_t) const noexcept;

            int compare(const UUID&) const noexcept;

            bool isEmpty() const noexcept;
            bool isNil() const noexcept;

            const SystemUUIDType& getSystemType() const noexcept;

            std::size_t hash() const noexcept;

            static Maybe<UUID> parse(const std::string&) noexcept;

        private:
            SystemUUIDType m_internal_uuid;

            friend std::istream& operator>>(std::istream&, UUID&) noexcept;
    };

    class HashOnlyUUID: public HMDT::UUID {
        public:
            HashOnlyUUID(std::size_t);

        private:
            std::size_t m_hash;

            friend std::hash<HashOnlyUUID>;
    };

    extern const UUID EMPTY_UUID;

    std::ostream& operator<<(std::ostream&, const UUID&) noexcept;
    std::istream& operator>>(std::istream&, UUID&) noexcept;
}

namespace std {
    template<>
    struct hash<HMDT::UUID> {
        std::size_t operator()(const HMDT::UUID& uuid) const noexcept {
            // NOTE: Do not use the Win32 UuidHash function because we want to
            //   ensure consistency of the calculated hash values on both Linux
            //   and Windows

            // https://stackoverflow.com/a/37152984
            const uint64_t* halves = reinterpret_cast<const uint64_t*>(&uuid.getSystemType());
            return halves[0] ^ halves[1];
        }
    };

    template<>
    struct hash<HMDT::HashOnlyUUID> {
        std::size_t operator()(const HMDT::HashOnlyUUID& uuid) const noexcept {
            return uuid.m_hash;
        }
    };

    string to_string(const HMDT::UUID&);
}

#endif

