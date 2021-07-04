/**
 * @file main.cpp
 *
 * @brief The starting point of the program
 */

#include <iostream>

#include "ArgParser.h"
#include "Options.h"

#include "Logger.h"
#include "ConsoleOutputFunctions.h"

#include "Interfaces.h"

MapNormalizer::ProgramOptions MapNormalizer::prog_opts;

/**
 * @brief The starting point of the program.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 *
 * @return 1 upon failure, 0 upon success.
 */
int main(int argc, char** argv) {
    std::ios_base::sync_with_stdio(false);
    std::cout.setf(std::ios::unitbuf);

    // First, we must register the console output function
    bool* quiet = nullptr;
    bool* verbose = nullptr;
    MapNormalizer::Log::Logger::registerOutputFunction(
        [&quiet, &verbose](const MapNormalizer::Log::Message& message) -> bool {
            bool is_quiet = quiet != nullptr && *quiet;
            bool is_verbose = verbose != nullptr && *verbose;

            const auto& level = message.getDebugLevel();

            // Debug only outputs if verbose is true
            if(!is_verbose && level == MapNormalizer::Log::Message::Level::DEBUG)
            {
                return true;
            }

            // Stdout and Debug only output if quiet is false
            // No need to check for debug because quiet and verbose cannot both
            //  be true at the same time
            if(is_quiet && level == MapNormalizer::Log::Message::Level::STDOUT)
            {
                return true;
            }

            return MapNormalizer::Log::outputWithFormatting(message);
        });

    // Parse the command-line arguments
    MapNormalizer::prog_opts = MapNormalizer::parseArgs(argc, argv);

    // Figure out if we should stop now based on the status of the parsing
    switch(MapNormalizer::prog_opts.status) {
        case 1:
            return 1;
        case 2:
            return 0;
        case 0:
        default:
            break;
    }

    quiet = &MapNormalizer::prog_opts.quiet;
    verbose = &MapNormalizer::prog_opts.verbose;

    return MapNormalizer::runApplication();
}

