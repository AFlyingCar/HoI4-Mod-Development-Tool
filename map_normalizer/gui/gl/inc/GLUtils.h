#ifndef GLUTILS_H
# define GLUTILS_H

# include <functional>
# include <string>
# include <vector>

# include "Source.h"

namespace MapNormalizer::GUI::GL {
    std::vector<std::string> glEnumToStrings(uint32_t);
    uint32_t processNextGLError(const std::function<void(uint32_t)>&);
    uint32_t processAllGLErrors(const std::function<void(uint32_t)>&);

    uint32_t logErrors(const Log::Source&);
}

# define MN_LOG_GL_ERRORS() \
    MapNormalizer::GUI::GL::logErrors(MN_LOG_SOURCE())

#endif

