#include "StatusCodes.h"

#include "StatusCategory.h"

std::error_code HMDT::makeErrorCode(const StatusCode& code) {
    return std::error_code(static_cast<int>(code), getStatusCategory());
}

