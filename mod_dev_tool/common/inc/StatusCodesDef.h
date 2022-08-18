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

// Calls Y(SYMBOL, BASE_VALUE)
//   Defines the next grouping of status codes
// Calls X(SYMBOL, "Description")
//   Defines a single status code
# define HMDT_STATUS_CODES() \
    X(SUCCESS, "Success") \
    /* Project Error Codes */ \
    Y(PROJECT, 0x100) \
    X(PROJECT_VALIDATION_FAILED, "Project Validation Failed.") \
    /* Province Project Error Codes */ \
    Y(PROVINCE_PROJECT, 0x200) \
    X(PROVINCE_INVALID_STATE_ID, "The Province's StateID is invalid.") \
    X(PROVINCE_NOT_IN_STATE, "The Province is not in the state it says it's in.") \
    /* State Project Error Codes */ \
    Y(STATE_PROJECT, 0x300) \
    X(STATE_DOES_NOT_EXIST, "The state does not exist.") \

#endif

