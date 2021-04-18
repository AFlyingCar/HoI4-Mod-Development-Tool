/**
 * @file main.cpp
 *
 * @brief The starting point of the program
 */

#include "ArgParser.h"
#include "Options.h"

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

    return MapNormalizer::runApplication();
}

