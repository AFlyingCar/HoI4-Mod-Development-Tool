#include "Uuid.h"

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
    return compare(right) != 0;
}

bool HMDT::UUID::operator==(const UUID& right) const noexcept {
    return compare(right) == 0;
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

    return UuidCompare(&m_internal_uuid, &right.getSystemType(), &status);
#else
    return uuid_compare(m_internal_uuid, right.getSystemType());
#endif
}

bool HMDT::UUID::isEmpty() const noexcept {
#ifdef WIN32
    [[maybe_unused]] RPC_STATUS status;
    return UuidIsNil(&m_internal_uuid, &status) == TRUE;
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
    if(m_hash.has_value()) {
        return m_hash.value();
    } else {
        return std::hash<HMDT::UUID>()(*this);
    }
}

std::size_t HMDT::UUID::hash() noexcept {
    if(!m_hash.has_value()) {
        m_hash = std::hash<HMDT::UUID>()(*this);
    }

    return m_hash.value();
}

HMDT::UUID HMDT::UUID::parse(const std::string& str) {
    UUID uuid(EMPTY_UUID);

    auto status = 
#ifdef WIN32
        UuidFromStringA(str.c_str(), &uuid.m_internal_uuid);
    constexpr auto FAILURE_STATUS = RPC_S_INVALID_STRING_UUID;
#else
        uuid_parse(str.c_str(), uuid.m_internal_uuid);
    constexpr auto FAILURE_STATUS = -1;
#endif

    if(status == FAILURE_STATUS) {
        WRITE_ERROR("Failed to parse uuid: ", str);
        // TODO: RETURN_IF_ERROR?
    }

    return uuid;
}

std::optional<std::size_t>& HMDT::UUID::getCachedHash() {
    return m_hash;
}

std::ostream& HMDT::operator<<(std::ostream& out, const UUID& uuid) noexcept {
    return out << std::to_string(uuid);

    return out;
}

std::istream& HMDT::operator>>(std::istream& in, UUID& uuid) noexcept {
    std::string str;
    // TODO: Read from in?

    uuid = UUID::parse(str);

    return in;
}

HMDT::HashOnlyUUID::HashOnlyUUID(std::size_t hash): UUID() {
    getCachedHash() = hash;
}

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

