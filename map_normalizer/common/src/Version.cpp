
#include "Version.h"

#include <sstream>

MapNormalizer::Version::Version(): m_version()
{ }

MapNormalizer::Version::Version(const Version& other):
    m_version(other.m_version)
{ }

MapNormalizer::Version::Version(Version&& other):
    m_version(std::move(other.m_version))
{ }

MapNormalizer::Version::Version(const std::string& version):
    m_version(version)
{ }

MapNormalizer::Version& MapNormalizer::Version::operator=(const Version& other)
{
    m_version = other.m_version;

    return *this;
}

bool MapNormalizer::Version::operator==(const Version& other) const {
    return compare(other) == 0;
}

bool MapNormalizer::Version::operator!=(const Version& other) const {
    return compare(other) != 0;
}

bool MapNormalizer::Version::operator<(const Version& other) const {
    return compare(other) < 0;
}

bool MapNormalizer::Version::operator>(const Version& other) const {
    return compare(other) > 0;
}

bool MapNormalizer::Version::operator<=(const Version& other) const {
    return compare(other) <= 0;
}

bool MapNormalizer::Version::operator>=(const Version& other) const {
    return compare(other) >= 0;
}

/**
 * @brief Gets the internal string for this version
 *
 * @return 
 */
const std::string& MapNormalizer::Version::str() const {
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
int MapNormalizer::Version::compare(const Version& other) const {
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

