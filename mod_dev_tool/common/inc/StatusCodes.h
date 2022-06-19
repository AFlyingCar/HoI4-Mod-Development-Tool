#ifndef STATUS_CODES_H
# define STATUS_CODES_H

# include <system_error>

# include "StatusCategory.h"
# include "PreprocessorUtils.h"

# include "StatusCodesDef.h"

namespace HMDT {
# define X(SYMBOL, VALUE, DESCRIPTION) \
    SYMBOL = VALUE,

    /**
     * @brief An enum representation of each available status code
     */
    enum class StatusCode {
        HMDT_STATUS_CODES()
    };

# undef X

    std::error_code makeErrorCode(const StatusCode&);

# define X(SYMBOL, VALUE, DESCRIPTION) \
    static inline const std::error_code CONCAT(STATUS_, SYMBOL) = makeErrorCode(StatusCode:: SYMBOL);

    HMDT_STATUS_CODES()

# undef X
}

#endif

