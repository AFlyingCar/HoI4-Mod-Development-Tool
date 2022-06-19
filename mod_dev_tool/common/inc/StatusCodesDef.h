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

// Calls X(SYMBOL, VALUE, "Description")
# define HMDT_STATUS_CODES()                                                   \
    X(SUCCESS, 0, "Success")                                                   \

#endif

