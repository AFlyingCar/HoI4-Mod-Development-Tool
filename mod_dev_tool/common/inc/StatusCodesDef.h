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
    X(SUCCESS, gettext("Success")) \
    X(BADALLOC, gettext("Memory allocation failed.")) \
    X(NOT_IMPLEMENTED, gettext("Feature is not implemented yet.")) \
    X(PARAM_CANNOT_BE_NULL, gettext("Given parameter may not be null.")) \
    X(VALUE_NOT_FOUND, gettext("The requested value could not be found.")) \
    X(UNINITIALIZED, gettext("The value has not been initialized or set yet.")) \
    X(VALIDATION_FAILED, gettext("Validation failed.")) \
    X(INVALID_TYPE, gettext("The given type is not valid.")) \
    X(KEY_EXISTS, gettext("The key already exists.")) \
    X(KEY_NOT_FOUND, gettext("The key does not exist.")) \
    /* Project Error Codes */ \
    Y(PROJECT, 0x100) \
    X(PROJECT_VALIDATION_FAILED, gettext("Project Validation Failed.")) \
    X(NO_PROJECT_LOADED, gettext("No project is currently loaded.")) \
    X(NO_DATA_LOADED, gettext("No data is currently loaded.")) \
    X(DIMENSION_MISMATCH, gettext("The loaded map image has dimensions which do not match previously loaded maps.")) \
    X(CALLBACK_NOT_REGISTERED, gettext("No prompt callback was registered with this project.")) \
    /* Province Project Error Codes */ \
    Y(PROVINCE_PROJECT, 0x200) \
    X(PROVINCE_INVALID_STATE_ID, gettext("The Province's StateID is invalid.")) \
    X(PROVINCE_NOT_IN_STATE, gettext("The Province is not in the state it says it's in.")) \
    /* State Project Error Codes */ \
    Y(STATE_PROJECT, 0x300) \
    X(STATE_DOES_NOT_EXIST, gettext("The state does not exist.")) \
    /* Gui Error Codes */ \
    Y(GUI, 0x2000) \
    X(DISPATCHER_DOES_NOT_EXIST, gettext("The provided dispatcher id does not exist.")) \
    X(ITEM_ADD_FAILURE, gettext("Failed to add item.")) \
    X(ITEM_BITMAP_READ_FAILURE, gettext("Failed to read bitmap when adding item.")) \
    X(NO_SUCH_ITEM_TYPE, gettext("No such item type of the given name exists.")) \
    X(EXPECTED_PATHS, gettext("The path list for adding an item is empty.")) \
    X(UNEXPECTED_RESPONSE, gettext("The response returned from the user prompt was unexpected.")) \
    /* ShapeFinder Error Codes */ \
    Y(SHAPEFINDER, 0x5000) \
    X(SHAPEFINDER_ESTOP, gettext("Shape Finder was stopped early.")) \
    /* File Error Codes */ \
    Y(FILECODES, 0x10000) \
    X(CANNOT_READ_FROM_STREAM, gettext("Unable to read from the given stream.")) \
    X(READ_TOO_FEW_BYTES, gettext("Too few bytes were read from the given stream.")) \
    /* Logger Error Codes */ \
    Y(LOGGER, 0x16000) \
    X(INVALID_LEVEL_STRING, gettext("String is unable to be converted to a level enum.")) \
    /* BitMap Error Codes */ \
    Y(BITMAP, 0x16200) \
    X(BITMAP_OFFSET_VALIDATION_ERROR, gettext("Validation using the BitMap's offset failed.")) \
    X(INVALID_BITS_PER_PIXEL, gettext("Invalid Bits Per Pixel.")) \
    X(COLOR_TABLE_REQUIRED, gettext("A color table is required to be provided.")) \
    X(INVALID_BIT_DEPTH, gettext("The bit-depth of the image is invalid.")) \
    /* Unexpected/Miscellaneous Error Codes */ \
    Y(MISCELLANEOUS, 0x7fffff9c) /* give us at least 100 before the end of the value space */ \
    X(UNEXPECTED, gettext("An unexpected error has occurred.")) \
    X(RECURSION_TOO_DEEP, gettext("Recursive function proceeded too deep.")) \

#endif

