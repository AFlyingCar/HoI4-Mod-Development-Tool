#include "StatusCodes.h"

#include "StatusCategory.h"

/**
 * @brief Creates a std::error_code for the given StatusCode
 *
 * @param code The StatusCode to create a std::error_code for
 *
 * @return A std::error_code representing the given StatusCode
 */
std::error_code HMDT::makeErrorCode(const StatusCode& code) {
    return std::error_code(static_cast<int>(code), getStatusCategory());
}

