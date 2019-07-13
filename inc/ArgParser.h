#ifndef ARG_PARSER_H
# define ARG_PARSER_H

#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <map>

namespace MapNormalizer {
    struct ProgramOptions {
        int status;

        std::string infilename;
        std::string outpath;
        bool verbose;
        bool quiet;
    };

    ProgramOptions parseArgs(int, char**);

    void printHelp();
}

#endif

