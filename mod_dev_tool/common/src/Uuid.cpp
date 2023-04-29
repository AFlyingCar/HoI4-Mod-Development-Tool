#include "Uuid.h"

#include <cstring>

#include "Logger.h"

const HMDT::UUID HMDT::EMPTY_UUID(HMDT::UUID::CreationParams::EMPTY);

HMDT::UUID::UUID(CreationParams params) {
    switch(params) {
        case CreationParams::EMPTY:
#ifdef WIN32
            (void)UuidCreateNil(&m_internal_uuid);
#else
            uuid_clear(m_internal_uuid);
#endif
            break;
        case CreationParams::RANDOM:
#ifndef WIN32
            uuid_generate_random(m_internal_uuid);
            break;
#endif
        [[fallthrough]];
        case CreationParams::TIME:
#ifndef WIN32
            uuid_generate_time(m_internal_uuid);
            break;
#endif
        [[fallthrough]];
        case CreationParams::BEST:
#ifdef WIN32
            (void)UuidCreate(&m_internal_uuid);
#else
            uuid_generate(m_internal_uuid);
#endif
            break;
    }
}

HMDT::UUID::UUID(const UUID& other) {
    *this = other;
}

HMDT::UUID::UUID(UUID&& other)
{
    *this = other;
}

HMDT::UUID& HMDT::UUID::operator=(const UUID& other) noexcept {
#ifdef WIN32
    std::memcpy(&m_internal_uuid, &other.m_internal_uuid, sizeof(SystemUUIDType));
#else
    uuid_copy(m_internal_uuid, other.m_internal_uuid);
#endif

    return *this;
}

HMDT::UUID::~UUID() { }

bool HMDT::UUID::operator!=(const UUID& right) const noexcept {
    return !(*this == right);
}

bool HMDT::UUID::operator==(const UUID& right) const noexcept {
    return hash() == right.hash();
}

bool HMDT::UUID::operator<(const UUID& right) const noexcept {
    return compare(right) < 0;
}

bool HMDT::UUID::operator<=(const UUID& right) const noexcept {
    return (*this < right) || (*this == right);
}

bool HMDT::UUID::operator>(const UUID& right) const noexcept {
    return compare(right) > 0;
}

bool HMDT::UUID::operator>=(const UUID& right) const noexcept {
    return (*this > right) || (*this == right);
}

/**
 * @brief Special case which returns true if this UUID has the requested hash.
 *
 * @param _hash The hash to compare against.
 *
 * @return True if the provided hash matches this UUID's hash, false otherwise.
 */
bool HMDT::UUID::operator==(std::size_t _hash) const noexcept {
    return hash() == _hash;
}

/**
 * @brief Compares two UUIDs.
 *
 * @param right The UUID to compare against.
 *
 * @return -1 if this UUID is less than 'right', 1 if this UUID is greater than
 *         right, or 0 if they are equal.
 */
int HMDT::UUID::compare(const UUID& right) const noexcept {
#ifdef WIN32
    [[maybe_unused]] RPC_STATUS status;

    // Note: For some stupid reason, the Win32 API takes the Uuid types by
    //   non-const pointer rather than by const pointer. So, make sure we cast
    //   away the const-ness before calling this function.
    return UuidCompare(
            const_cast<SystemUUIDType*>(&m_internal_uuid),
            const_cast<SystemUUIDType*>(&right.getSystemType()),
            &status);
#else
    return uuid_compare(m_internal_uuid, right.getSystemType());
#endif
}

bool HMDT::UUID::isEmpty() const noexcept {
#ifdef WIN32
    [[maybe_unused]] RPC_STATUS status;
    return UuidIsNil(
            const_cast<SystemUUIDType*>(&m_internal_uuid),
            &status) == TRUE;
#else
    return uuid_is_null(m_internal_uuid) == 1;
#endif
}

bool HMDT::UUID::isNil() const noexcept {
    return isEmpty();
}

auto HMDT::UUID::getSystemType() const noexcept -> const SystemUUIDType&
{
    return m_internal_uuid;
}

std::size_t HMDT::UUID::hash() const noexcept {
    return std::hash<HMDT::UUID>()(*this);
}

HMDT::Maybe<HMDT::UUID> HMDT::UUID::parse(const std::string& str) noexcept {
    UUID uuid(EMPTY_UUID);

#ifdef WIN32
    auto status = UuidFromStringA(
            static_cast<RPC_CSTR>(str.c_str()),
            const_cast<SystemUUIDType*>(&uuid.m_internal_uuid));
    constexpr auto FAILURE_STATUS = RPC_S_INVALID_STRING_UUID;
#else
    auto status = uuid_parse(str.c_str(), uuid.m_internal_uuid);
    constexpr auto FAILURE_STATUS = -1;
#endif

    if(status == FAILURE_STATUS) {
        WRITE_ERROR("Failed to parse uuid: ", str);
        // TODO: RETURN_IF_ERROR?
    }

    return uuid;
}

std::ostream& HMDT::operator<<(std::ostream& out, const UUID& uuid) noexcept {
    return out << std::to_string(uuid);

    return out;
}

/**
 * @brief Parses a UUID out of an istream. Will not modify 'uuid' if parsing
 *        fails.
 *
 * @param in The input stream.
 * @param uuid The uuid to parse the results into.
 *
 * @return in
 */
std::istream& HMDT::operator>>(std::istream& in, UUID& uuid) noexcept {
    std::string str;

    // Read the first token, it should be a full UUID.
    // If the stream contains data like "{UUID}extradata", then it is not to be
    //   considered a valid UUID
    in >> str;

    if(str.size() < UUID::STRING_REPR_LENGTH) {
        WRITE_ERROR("Incorrect string length ", str.size(), ", expected ",
                    UUID::STRING_REPR_LENGTH, ". Unable to parse '", str, '\'');
        return in;
    }

    auto new_uuid = UUID::parse(str);

    if(IS_FAILURE(new_uuid)) {
        WRITE_ERROR("Failed to parse UUID of '", str, '\'');
        return in;
    }

    // Write the data into 'uuid' now that we know it parsed successfully.
    uuid = *new_uuid;

    return in;
}

HMDT::HashOnlyUUID::HashOnlyUUID(std::size_t hash):
    UUID(CreationParams::EMPTY),
    m_hash(hash)
{ }

std::string std::to_string(const HMDT::UUID& uuid) {
#ifdef WIN32
    unsigned char* str;
    UuidToStringA(&uuid.getSystemType(), &str);

    std::string s((char*)str);

    RpcStringFreeA(&str);
#else

    char s[37];
    uuid_unparse(uuid.getSystemType(), s);
#endif

    return s;
}

