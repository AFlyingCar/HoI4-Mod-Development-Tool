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
    X(BADALLOC, "Memory allocation failed.") \
    X(NOT_IMPLEMENTED, "Feature is not implemented yet.") \
    X(PARAM_CANNOT_BE_NULL, "Given parameter may not be null.") \
    /* Project Error Codes */ \
    Y(PROJECT, 0x100) \
    X(PROJECT_VALIDATION_FAILED, "Project Validation Failed.") \
    X(NO_PROJECT_LOADED, "No project is currently loaded.") \
    X(NO_DATA_LOADED, "No data is currently loaded.") \
    /* Province Project Error Codes */ \
    Y(PROVINCE_PROJECT, 0x200) \
    X(PROVINCE_INVALID_STATE_ID, "The Province's StateID is invalid.") \
    X(PROVINCE_NOT_IN_STATE, "The Province is not in the state it says it's in.") \
    /* State Project Error Codes */ \
    Y(STATE_PROJECT, 0x300) \
    X(STATE_DOES_NOT_EXIST, "The state does not exist.") \
    /* Gui Error Codes */ \
    Y(GUI, 0x2000) \
    X(DISPATCHER_DOES_NOT_EXIST, "The provided dispatcher id does not exist.") \
    X(ITEM_ADD_FAILURE, "Failed to add item.") \
    X(ITEM_BITMAP_READ_FAILURE, "Failed to read bitmap when adding item.") \
    X(NO_SUCH_ITEM_TYPE, "No such item type of the given name exists.") \
    X(EXPECTED_PATHS, "The path list for adding an item is empty.") \
    /* ShapeFinder Error Codes */ \
    Y(SHAPEFINDER, 0x5000) \
    X(SHAPEFINDER_ESTOP, "Shape Finder was stopped early.") \
    /* File Error Codes */ \
    Y(FILECODES, 0x10000) \
    X(CANNOT_READ_FROM_STREAM, "Unable to read from the given stream.") \
    X(READ_TOO_FEW_BYTES, "Too few bytes were read from the given stream.") \
    /* Logger Error Codes */ \
    Y(LOGGER, 0x16000) \
    X(INVALID_LEVEL_STRING, "String is unable to be converted to a level enum.") \
    /* BitMap Error Codes */ \
    Y(BITMAP, 0x16200) \
    X(BITMAP_OFFSET_VALIDATION_ERROR, "Validation using the BitMap's offset failed.") \
    X(INVALID_BITS_PER_PIXEL, "Invalid Bits Per Pixel.") \
    X(COLOR_TABLE_REQUIRED, "A color table is required to be provided.") \
    X(INVALID_BIT_DEPTH, "The bit-depth of the image is invalid.") \

#endif

