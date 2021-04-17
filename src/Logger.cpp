#include "Logger.h"

#include <iostream>
#include <vector>
#include <stdio.h>

#include <unistd.h>

#include "Options.h"

static std::string info_line;

/**
 * @brief Checks if we can display ANSI escape codes through stdout.
 * @details Specifically, checks if the STDOUT file is a TTY.
 *
 * @return True if we can display ANSI through stdout, false otherwise.
 */
static bool isOutAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stdout));

    return ansi_enabled;
}

/**
 * @brief Checks if we can display ANSI escape codes through stderr.
 * @details Specifically, checks if the STDERR file is a TTY.
 *
 * @return True if we can display ANSI through stderr, false otherwise.
 */
static bool isErrAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stderr));

    return ansi_enabled;
}

/**
 * @brief Outputs the current information line.
 * @details Will do nothing if program output is set to QUIET
 */
static void writeInfoLine() {
    if(MapNormalizer::prog_opts.quiet) return;

    if(!info_line.empty()) {
        std::cout << (isOutAnsiEnabled() ? "\33[32m" : "") << "==> " << info_line
                  << (isOutAnsiEnabled() ? "\33[0m" : "")
                  << (!isOutAnsiEnabled() ? "\n" : "");
    }
}

/**
 * @brief Initializes the logging system
 */
void MapNormalizer::init() {
    // https://stackoverflow.com/a/61854612
    std::ios_base::sync_with_stdio(false);
}

/**
 * @brief Deletes the last information line from the console, so that we can display the next one.
 * @details Will do nothing if program output is set to QUIET or if ANSI escape
 *          codes are not enabled for STDOUT.
 */
void MapNormalizer::deleteInfoLine() {
    if(prog_opts.quiet) return;

    if(!info_line.empty() && isOutAnsiEnabled()) {
        std::cout << "\33[1000D"  // Go to start of line
                     "\33[0K"     // Clear the line
                     "\33[1000D"; // Go to start of line
    }
}

/**
 * @brief Sets and displays the next information line.
 *
 * @param line The next textual string to use for the information line.
 */
void MapNormalizer::setInfoLineImpl(std::string&& line) {
    if(MapNormalizer::prog_opts.quiet) return;

    info_line = std::move(line);

    if(!info_line.empty() && isOutAnsiEnabled()) {
        std::cout << "\33[1000D" // Go to start of line
                     "\33[0K"    // Clear the line
                     "\33[1000D" // Go to start of line
                     "\33[32m==> " << info_line; // Write the line
    } else {
        std::cout << info_line << "\n";
    }
}

/**
 * @brief Writes a warning.
 * @details Outputs to stderr. If requested, prefix is "[WRN] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeWarningImpl(const std::string& message,
                                     bool write_prefix)
{
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[33m" : "")
              << (write_prefix ? "[WRN] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

/**
 * @brief Writes an error.
 * @details Outputs to stderr. If requested, prefix is "[ERR] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeErrorImpl(const std::string& message,
                                   bool write_prefix)
{
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[31m" : "")
              << (write_prefix ? "[ERR] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

/**
 * @brief Writes a output message.
 * @details Outputs to stdout. If requested, prefix is "[OUT] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeStdoutImpl(const std::string& message,
                                    bool write_prefix)
{
    if(prog_opts.quiet) return;

    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[37m" : "")
              << (write_prefix ? "[OUT] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

/**
 * @brief Writes a debug message.
 * @details Outputs to stdout. If requested, prefix is "[DBG] ~ ".
 *
 * @param message The message to write.
 * @param write_prefix Whether a prefix should be attached to this message.
 */
void MapNormalizer::writeDebugImpl(const std::string& message,
                                   bool write_prefix)
{
    // Do nothing if we don't have verbose output.
    if(!prog_opts.verbose) return;

    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[34m" : "")
              << (write_prefix ? "[DBG] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    writeInfoLine();
}

