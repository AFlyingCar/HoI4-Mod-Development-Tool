/**
 * @file GLUtils.h
 *
 * @brief Defines OpenGL utility functions and macros.
 */

#ifndef GLUTILS_H
# define GLUTILS_H

# include <functional>
# include <string>
# include <vector>

# include "Source.h"

namespace HMDT::GUI::GL {
    std::vector<std::string> glEnumToStrings(uint32_t);
    uint32_t processNextGLError(const std::function<void(uint32_t)>&);
    uint32_t processAllGLErrors(const std::function<void(uint32_t)>&);

    uint32_t logErrors(const Log::Source&);
}

/**
 * @brief Wrapper around logErrors which passes in a Log::Source object.
 */
# define HMDT_LOG_GL_ERRORS() HMDT::GUI::GL::logErrors(HMDT_LOG_SOURCE())

#endif

