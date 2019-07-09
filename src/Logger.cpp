#include "Logger.h"

#include <iostream>
#include <vector>
#include <stdio.h>

#include <unistd.h>

static std::string info_line;

static bool isOutAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stdout));

    return ansi_enabled;
}

static bool isErrAnsiEnabled() {
    static bool ansi_enabled = isatty(fileno(stderr));

    return ansi_enabled;
}

static void writeInfoLine() {
    if(!info_line.empty()) {
        std::cout << (isOutAnsiEnabled() ? "\33[32m" : "") << "==> " << info_line
                  << (isOutAnsiEnabled() ? "\33[0m" : "");
        if(!isOutAnsiEnabled()) std::cout << std::endl;
        std::cout.flush();
    }
}

void MapNormalizer::deleteInfoLine() {
    if(!info_line.empty() && isOutAnsiEnabled()) {
        std::cout << "\33[1000D"; // Go to start of line
        std::cout << "\33[0K";    // Clear the line
        std::cout << "\33[1000D"; // Go to start of line
        std::cout.flush();
    }
}

void MapNormalizer::setInfoLine(const std::string& line) {
    info_line = line;

    deleteInfoLine();
    writeInfoLine();
}


void MapNormalizer::writeWarning(const std::string& message, bool write_prefix)
{
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[33m" : "")
              << (write_prefix ? "[WRN] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cerr.flush();
    writeInfoLine();
}

void MapNormalizer::writeError(const std::string& message, bool write_prefix) {
    deleteInfoLine();
    std::cerr << (isErrAnsiEnabled() ? "\33[31m" : "")
              << (write_prefix ? "[ERR] ~ " : "") << message
              << (isErrAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cerr.flush();
    writeInfoLine();
}

void MapNormalizer::writeStdout(const std::string& message, bool write_prefix) {
    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[37m" : "")
              << (write_prefix ? "[OUT] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cout.flush();
    writeInfoLine();
}

void MapNormalizer::writeDebug(const std::string& message, bool write_prefix) {
    deleteInfoLine();
    std::cout << (isOutAnsiEnabled() ? "\33[34m" : "")
              << (write_prefix ? "[DBG] ~ " : "") << message
              << (isOutAnsiEnabled() ? "\33[0m" : "") << std::endl;
    std::cout.flush();
    writeInfoLine();
}

