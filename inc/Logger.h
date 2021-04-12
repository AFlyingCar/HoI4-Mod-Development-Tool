/**
 * @file Logger.h
 *
 * @brief Defines all functions used for logging information.
 */

#ifndef LOGGER_H
# define LOGGER_H

# include <string>
# include <sstream>


namespace MapNormalizer {
    void deleteInfoLine();
    void setInfoLineImpl(const std::string&);
    void writeErrorImpl(const std::string&, bool = true);
    void writeWarningImpl(const std::string&, bool = true);
    void writeStdoutImpl(const std::string&, bool = true);
    void writeDebugImpl(const std::string&, bool = true);

    template<typename... Args>
    void setInfoLine(const Args&... args) {
        std::stringstream ss;
        (ss << ... << args);
        setInfoLineImpl(ss.str());
    }

    template<bool WritePrefix = true, typename... Args>
    void writeError(const Args&... args) {
        std::stringstream ss;
        (ss << ... << args);
        writeErrorImpl(ss.str(), WritePrefix);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeWarning(const Args&... args) {
        std::stringstream ss;
        (ss << ... << args);
        writeWarningImpl(ss.str(), WritePrefix);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeStdout(const Args&... args) {
        std::stringstream ss;
        (ss << ... << args);
        writeStdoutImpl(ss.str(), WritePrefix);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeDebug(const Args&... args) {
        std::stringstream ss;
        (ss << ... << args);
        writeDebugImpl(ss.str(), WritePrefix);
    }
}

#endif

