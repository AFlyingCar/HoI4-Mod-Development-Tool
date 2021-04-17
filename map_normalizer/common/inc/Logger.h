/**
 * @file Logger.h
 *
 * @brief Defines all functions used for logging information.
 */

#ifndef LOGGER_H
# define LOGGER_H

# include <string>
# include <sstream>

# include "Options.h"

namespace {
    std::stringstream s_log_builder;
}

namespace MapNormalizer {
    void init();

    void deleteInfoLine();
    void setInfoLineImpl(std::string&&);
    void writeErrorImpl(const std::string&, bool = true);
    void writeWarningImpl(const std::string&, bool = true);
    void writeStdoutImpl(const std::string&, bool = true);
    void writeDebugImpl(const std::string&, bool = true);

    template<typename... Args>
    void setInfoLine(const Args&... args) {
        if(MapNormalizer::prog_opts.quiet) return;

        s_log_builder.str(std::string{});
        (s_log_builder << ... << args);
        setInfoLineImpl(s_log_builder.str());
    }

    template<bool WritePrefix = true, typename... Args>
    void writeError(const Args&... args) {
        s_log_builder.str(std::string{});
        (s_log_builder << ... << args);
        writeErrorImpl(s_log_builder.str(), WritePrefix);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeWarning(const Args&... args) {
        s_log_builder.str(std::string{});
        (s_log_builder << ... << args);
        writeWarningImpl(s_log_builder.str(), WritePrefix);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeStdout(const Args&... args) {
        if(prog_opts.quiet) return;

        s_log_builder.str(std::string{});
        (s_log_builder << ... << args);
        writeStdoutImpl(s_log_builder.str(), WritePrefix);
    }

    template<bool WritePrefix = true, typename... Args>
    void writeDebug(const Args&... args) {
        if(!prog_opts.verbose) return;

        s_log_builder.str(std::string{});
        (s_log_builder << ... << args);
        writeDebugImpl(s_log_builder.str(), WritePrefix);
    }
}

#endif

