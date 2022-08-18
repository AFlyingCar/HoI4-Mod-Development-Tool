/**
 * @file StatusCodesDef.h
 *
 * @brief Defines every status code that will be used.
 *
 * @details To add new status codes, simply add a new entry to the
 *          HMDT_STATUS_CODES x-macro. Make sure that no VALUE fields overlap,
 *          as aliased status codes are _NOT_ supported.
 */

#ifndef STATUS_CODES_DEF_H
# define STATUS_CODES_DEF_H

# include <system_error>

// Calls X(SYMBOL, VALUE, "Description")
# define HMDT_STATUS_CODES()                                                   \
    X(SUCCESS, 0, "Success")                                                   \
                                                                               \
    /* Project Error Codes */                                                  \
    X(PROJECT_VALIDATION_FAILED, 1, "Project Validation Failed.")              \
    X(PROVINCE_INVALID_STATE_ID, 2, "The Province's StateID is invalid.")      \
    X(PROVINCE_NOT_IN_STATE, 3, "The Province is not in the state it says it's in.") \
                                                                               \
    /* State Error Codes */                                                    \
    X(STATE_DOES_NOT_EXIST, 1024, "The state does not exist.")                 \

#endif

