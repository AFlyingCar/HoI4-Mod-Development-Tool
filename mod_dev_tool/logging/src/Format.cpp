
#include "Format.h"

#include <cstring>

/**
 * @brief Combines two Format objects together
 * @details The type is concatenated together as a bit-field, but only the data
 *          field of 'other' is copied over
 *
 * @param other The other Format object
 *
 * @return A new Format object containing a combined type and the data of 'other'
 */
auto HMDT::Log::Format::operator|(const Format& other) const -> Format {
    Format f { static_cast<Type>(type | other.type) };

    std::memcpy(f.data, other.data, DATA_SIZE);

    return f;
}

/**
 * @brief Compares two Format objects
 *
 * @param other The other Format object
 *
 * @return true if they are equivalent, false otherwise
 */
bool HMDT::Log::Format::operator==(const Format& other) const {
    return type == other.type && std::memcmp(data, other.data, DATA_SIZE) == 0;
}

