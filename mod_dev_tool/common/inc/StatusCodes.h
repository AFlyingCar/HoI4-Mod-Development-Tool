#ifndef STATUS_CODES_H
# define STATUS_CODES_H

# include <system_error>

# include "StatusCategory.h"
# include "PreprocessorUtils.h"

# include "StatusCodesDef.h"

namespace HMDT {
# define Y(SYMBOL, BASE_VALUE) \
    SYMBOL = BASE_VALUE,

# define X(SYMBOL, VALUE, DESCRIPTION) \
    SYMBOL = VALUE,

    /**
     * @brief An enum representation of each available status code
     */
    enum class StatusCode {
        HMDT_STATUS_CODES()
    };

# undef X
# undef Y

    std::error_code makeErrorCode(const StatusCode&);

# define Y(...)
# define X(SYMBOL, VALUE, DESCRIPTION) \
    static inline const std::error_code CONCAT(STATUS_, SYMBOL) = makeErrorCode(StatusCode:: SYMBOL);

    HMDT_STATUS_CODES()

# undef X
# undef Y
}

#endif

