#ifndef STATUS_CODES_H
# define STATUS_CODES_H

# include <system_error>

# include "StatusCategory.h"
# include "PreprocessorUtils.h"

# include "StatusCodesDef.h"

namespace HMDT {
# define Y(SYMBOL, BASE_VALUE) \
    CONCAT(SYMBOL, _CODES) = BASE_VALUE,

# define X(SYMBOL, DESCRIPTION) \
    SYMBOL,

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
# define X(SYMBOL,  DESCRIPTION) \
    static inline const std::error_code CONCAT(STATUS_, SYMBOL) = makeErrorCode(StatusCode:: SYMBOL);

    HMDT_STATUS_CODES()

# undef X
# undef Y
}

#endif

