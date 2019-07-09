#ifndef LOGGER_H
# define LOGGER_H

#include <string>

namespace MapNormalizer {
    void deleteInfoLine();
    void setInfoLine(const std::string&);
    void writeError(const std::string&, bool = true);
    void writeWarning(const std::string&, bool = true);
    void writeStdout(const std::string&, bool = true);
    void writeDebug(const std::string&, bool = true);
}

#endif

