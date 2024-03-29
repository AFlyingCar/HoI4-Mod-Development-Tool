
#include "StatusCategory.h"

#include <libintl.h>

#include "StatusCodes.h"
#include "Constants.h"
#include "PreprocessorUtils.h"

#include "Logger.h"

HMDT::StatusCategory::StatusCategory() noexcept { }

const char* HMDT::StatusCategory::name() const noexcept {
    return STATUS_CATEGORY_NAME.data();
}

std::error_condition HMDT::StatusCategory::default_error_condition(int code) const noexcept
{

#define Y(...)
#define X(SYMBOL, DESCRIPTION) \
    case StatusCode:: SYMBOL : return std::error_condition(code, *this);

    switch(static_cast<StatusCode>(code)) {
        HMDT_STATUS_CODES()

        // Note that we need this default _JUST IN CASE_ 'code' somehow is an
        //   invalid enum value.
        default:
            WRITE_ERROR("Invalid error code '", code,
                        "'. Representing with generic_category instead.");
            return std::error_condition(code, std::generic_category());
    }

#undef X
#undef Y
}

bool HMDT::StatusCategory::equivalent(const std::error_code& code,
                                      int condition) const noexcept
{
    return *this == code.category() && code.value() == condition;
}

bool HMDT::StatusCategory::equivalent(int code,
                                      const std::error_condition& condition) const noexcept
{
    return default_error_condition(code) == condition;
}

std::string HMDT::StatusCategory::message(int code) const {
#define Y(...)
#define X(SYMBOL, DESCRIPTION) \
    case StatusCode:: SYMBOL : return DESCRIPTION ;

    switch(static_cast<StatusCode>(code)) {
        HMDT_STATUS_CODES()

        // Note that we need this default _JUST IN CASE_ 'code' somehow is an
        //   invalid enum value.
        default:
            return std::generic_category().message(code);
    }

#undef X
}

const HMDT::StatusCategory& HMDT::getStatusCategory() {
    static StatusCategory category;

    return category;
}

