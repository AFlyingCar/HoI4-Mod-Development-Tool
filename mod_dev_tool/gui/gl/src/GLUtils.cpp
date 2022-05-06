/**
 * @file GLUtils.cpp
 *
 * @brief Defines OpenGL utility functions and macros.
 */

#include "GLUtils.h"

#include <GL/glew.h>

#include "GLXMacros.h"

#include "Logger.h"

/**
 * @brief Converts a GLenum value into a list of potential strings that it may
 *        have been generated from
 *
 * @param value The value to get the string for
 *
 * @return A list of strings of the potential names that the value may represent
 */
std::vector<std::string> HMDT::GUI::GL::glEnumToStrings(uint32_t value) {
    switch(value) {
/*
 * Empty string at the front is so we implicitly concatenate with whatever is in
 * __VA_ARGS__, which may or may not be empty
 */
#define X(GLENUM_NAME, GLENUM_VALUE, ...) \
        case GLENUM_NAME: return { #GLENUM_NAME, "" __VA_ARGS__ };
    HMDT_GL_XMACRO
#undef X
        default:
            return {"<INVALID>"};
    }
}

/**
 * @brief Processes the next GLenum which is not GL_NO_ERROR
 *
 * @param processor A function which can process a GLenum
 *
 * @return The last GLenum
 */
uint32_t HMDT::GUI::GL::processNextGLError(const std::function<void(uint32_t)>& processor)
{
    GLenum error = glGetError();

    if(error != GL_NO_ERROR) processor(error);

    return error;
}

/**
 * @brief Processes all currently waiting GL errors
 *
 * @param processor A function which can process a GLenum
 *
 * @return The number of GLenums that were processed
 */
uint32_t HMDT::GUI::GL::processAllGLErrors(const std::function<void(uint32_t)>& processor)
{
    uint32_t num_errors_processed = 0;
    for(; processNextGLError(processor) != GL_NO_ERROR; ++num_errors_processed);

    return num_errors_processed;
}

/**
 * @brief Logs all GLErrors using the logging facilities
 *
 * @param source The source location this function was called from.
 *
 * @return The number of errors logged
 */
uint32_t HMDT::GUI::GL::logErrors(const Log::Source& source) {
    return processAllGLErrors([&source](uint32_t error) {
        // Invoke the writeError() function manually so we can control the
        //  source object directly
        Log::Logger::getInstance().writeError(source, " GLError: ", error, " => ",
                                              HMDT::GUI::GL::glEnumToStrings(error).front());
    });
}

