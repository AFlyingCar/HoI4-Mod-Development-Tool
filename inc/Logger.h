#ifndef LOGGER_H
# define LOGGER_H

#include <string>

namespace MapNormalizer {
    void setInfoLine(const std::string&);
    void writeError(const std::string&);
    void writeWarning(const std::string&);
    void writeStdout(const std::string&);
}

#endif

