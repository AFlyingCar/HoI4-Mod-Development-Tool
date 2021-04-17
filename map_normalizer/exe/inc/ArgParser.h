/**
 * @file ArgParser.h
 *
 * @brief Defines the structure for storing global program options, and the
 *        functions for parsing command-line arguments.
 */

#ifndef ARG_PARSER_H
# define ARG_PARSER_H

# include <string>

# include "Options.h"

namespace MapNormalizer {
    ProgramOptions parseArgs(int, char**);

    void printHelp();
}

#endif

