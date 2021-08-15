#ifndef GLUTILS_H
# define GLUTILS_H

# include <functional>
# include <string>
# include <vector>

namespace MapNormalizer::GUI::GL {
    std::vector<std::string> glEnumToStrings(uint32_t);
    uint32_t processNextGLError(const std::function<void(uint32_t)>&);
    uint32_t processAllGLErrors(const std::function<void(uint32_t)>&);

    uint32_t logErrors(const std::string&, uint32_t);
}

// TODO: This should generate a Log::Source object, once the new logging system
//       is merged.
# define MN_LOG_GL_ERRORS() \
    MapNormalizer::GUI::GL::logErrors(__FILE__, __LINE__)

#endif

