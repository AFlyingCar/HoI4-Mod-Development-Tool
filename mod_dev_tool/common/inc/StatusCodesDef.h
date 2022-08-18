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

# define _MAKE_STATUS_CODE(BASE, V) \
    (static_cast<uint32_t>(BASE) | static_cast<uint32_t>(V))

// Calls Y(SYMBOL, BASE_VALUE)
//   Defines the next grouping of status codes
// Calls X(SYMBOL, VALUE, "Description")
//   Defines a single status code
# define HMDT_STATUS_CODES() \
    X(SUCCESS, 0, "Success") \
    /* Project Error Codes */ \
    Y(PROJECT_CODES, 0x100) \
    X(PROJECT_VALIDATION_FAILED, _MAKE_STATUS_CODE(PROJECT_CODES, 1), "Project Validation Failed.") \
    /* Province Project Error Codes */ \
    Y(PROVINCE_PROJECT_CODES, 0x200) \
    X(PROVINCE_INVALID_STATE_ID, _MAKE_STATUS_CODE(PROVINCE_PROJECT_CODES, 1), "The Province's StateID is invalid.") \
    X(PROVINCE_NOT_IN_STATE, _MAKE_STATUS_CODE(PROVINCE_PROJECT_CODES, 2), "The Province is not in the state it says it's in.") \
    /* State Project Error Codes */ \
    Y(STATE_PROJECT_CODES, 0x300) \
    X(STATE_DOES_NOT_EXIST, _MAKE_STATUS_CODE(STATE_PROJECT_CODES, 1), "The state does not exist.") \

#endif

