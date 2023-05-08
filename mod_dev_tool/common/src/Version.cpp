
#include "Version.h"

#include <sstream>

/**
 * @brief Constructs an empty Version
 */
HMDT::Version::Version(): m_version()
{ }

/**
 * @brief Copies an existing Version.
 *
 * @param other The other version.
 */
HMDT::Version::Version(const Version& other):
    m_version(other.m_version)
{ }

/**
 * @brief Moves another version into this one.
 *
 * @param other The other version.
 */
HMDT::Version::Version(Version&& other):
    m_version(std::move(other.m_version))
{ }

/**
 * @brief Constructs a new Version
 *
 * @param version The string-representation to construct this Version with
 */
HMDT::Version::Version(const std::string& version):
    m_version(version)
{ }

/**
 * @brief Assigns another Version to this Version
 *
 * @param other The other version
 *
 * @return This version
 */
HMDT::Version& HMDT::Version::operator=(const Version& other) {
    m_version = other.m_version;

    return *this;
}

/**
 * @brief Compares if this version is equal to another
 *
 * @param other The other version
 *
 * @return True if this version is equal to the other version
 */
bool HMDT::Version::operator==(const Version& other) const {
    return compare(other) == 0;
}

/**
 * @brief Compares if this version is not equal another.
 *
 * @param other The other version
 *
 * @return True if this version is not equal to the other version
 */
bool HMDT::Version::operator!=(const Version& other) const {
    return compare(other) != 0;
}

/**
 * @brief Compares if this version is smaller than other
 *
 * @param other The other version
 *
 * @return True if this version is smaller than other
 */
bool HMDT::Version::operator<(const Version& other) const {
    return compare(other) < 0;
}

/**
 * @brief Compares if this version is greater than other
 *
 * @param other The other version
 *
 * @return True if this version is greater than other
 */
bool HMDT::Version::operator>(const Version& other) const {
    return compare(other) > 0;
}

/**
 * @brief Compares if this version is smaller or equal to than other
 *
 * @param other The other version
 *
 * @return True if this version is smaller or equal to other
 */
bool HMDT::Version::operator<=(const Version& other) const {
    return compare(other) <= 0;
}

/**
 * @brief Compares if this version is greater or equal to than other
 *
 * @param other The other version
 *
 * @return True if this version is greater or equal to other
 */
bool HMDT::Version::operator>=(const Version& other) const {
    return compare(other) >= 0;
}

/**
 * @brief Gets the internal string for this version
 *
 * @return 
 */
const std::string& HMDT::Version::str() const {
    return m_version;
}

/**
 * @brief Compares two version numbers
 *
 * @param other The other version number
 *
 * @return 0 if the version numbers are the same, -1 if this version number is
 *         smaller than 'other', and 1 if this version number is bigger
 */
int HMDT::Version::compare(const Version& other) const {
    if(m_version == other.m_version) return 0;

    std::stringstream ss1(m_version);
    std::stringstream ss2(other.m_version);

    std::string part1;
    std::string part2;

    // Keep going until one of the two version numbers is empty
    while(std::getline(ss1, part1, '.') && std::getline(ss2, part2, '.')) {
        // TODO: How do we handle things like rc (release candidate), a, or b?
        int n1 = std::stoi(part1);
        int n2 = std::stoi(part2);

        if(n1 < n2) {
            return -1;
        }
    }
    
    // If the version numbers are of different lengths, then keep going for the
    //  other one, and interpret the missing version parts as 0
    if(!ss1) {
        while(std::getline(ss2, part2, '.')) {
            int n = std::stoi(part2);

            if(0 < n) {
                return -1;
            }
        }
    }

    // Note that there is no need to check if ss2 is empty, as that would
    //  essentially result in a bunch of comparisons of "n < 0", which should
    //  never be the case (version number parts cannot be negative)

    return 1;
}

/**
 * @brief Outputs a Version
 *
 * @param stream The stream to output to
 * @param version The version to output
 *
 * @return stream
 */
std::ostream& HMDT::operator<<(std::ostream& stream, const Version& version) {
    return (stream << version.str());
}

HMDT::Version HMDT::operator""_V(const char* version, size_t length) {
    return Version{ std::string(version, length) };
}
