/**
 * @file ArgParser.h
 *
 * @brief Defines the functions for parsing command-line arguments.
 */

#include "ArgParser.h"

#include <iostream>
#include <getopt.h>

#include "Logger.h"

//! The name of the program executable
static std::string program_name = "map_normalizer";

/**
 * @brief Prints help information to the console.
 */
void MapNormalizer::printHelp() {
    writeStdout(program_name + " [OPTIONS...] [INFILE] [OUTPATH]", false);
    writeStdout("\t   --no-gui                Do not open or render the map GUI.", false);
    writeStdout("\t   --no-skip-no-name-state Do not open or render the map GUI.", false);
    writeStdout("\t   --state-input           The input file for writing state definitions.", false);
    writeStdout("\t   --height-map            The input file for writing the normal map.", false);
    writeStdout("\t-v,--verbose               Display all output.", false);
    writeStdout("\t-q,--quiet                 Display only errors and warnings (does not affect this message).", false);
    writeStdout("\t-h,--help                  Display this message and exit.", false);
}

/**
 * @brief Parses command-line arguments.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 *
 * @return A structure containing global program options.
 */
auto MapNormalizer::parseArgs(int argc, char** argv) -> ProgramOptions {
    using namespace std::string_literals;

    // First, lets determine what the actual program name is
    ::program_name = argv[0];
    ::program_name = ::program_name.substr(::program_name.find_last_of('/') + 1);

    // Setup data for getopt
    static const char* const short_options = "vqh";
    static option long_options[] = {
        { "verbose", no_argument, NULL, 'v' },
        { "quiet", no_argument, NULL, 'q' },
        { "help", no_argument, NULL, 'h' },
        { "no-gui", no_argument, NULL, 1 },
        { "state-input", required_argument, NULL, 2 },
        { "height-map", required_argument, NULL, 3 },
        { "no-skip-no-name-state", no_argument, NULL, 4 },
        { nullptr, 0, nullptr, 0}
    };

    // Setup default option values
    ProgramOptions prog_opts { 0, "", "", false, false, false, "", "", false };

    int optindex = 0;
    int c = 0;

    // We have multiple required arguments, so if we have none then just
    //  fail immediately
    if(argc == 1) {
        printHelp();
        prog_opts.status = 1;
        return prog_opts;
    }

    // Supress getopt error reporting, so that we can handle it ourselves.
    opterr = 0;

    do {
        c = getopt_long(argc, argv, short_options, long_options, &optindex);

        // In case it is -1 on the first go around
        if(c == -1) break;

        switch(c) {
            case 0:
                // The user shouldn't be able to see this
                //   If it is possible to see, then whatever causes that to
                //   be seen should be fixed
                writeDebug("opterr = "s + std::to_string(opterr));
                writeDebug("optopt = "s + std::to_string(optopt));
                writeDebug("optind = "s + std::to_string(optind));
                if(optarg)
                    writeDebug("optarg = "s + optarg);
                else
                    writeDebug("optarg = 0x0");
                break;
            case 1: // --no-gui
                prog_opts.no_gui = true;
                break;
            case 2: // --state-input
                if(optarg == nullptr) {
                    writeWarning("Missing argument to option 'state-input'. Assuming no option.");
                    prog_opts.state_input_file = "";
                } else {
                    prog_opts.state_input_file = optarg;
                }
                break;
            case 3: // --height-map
                if(optarg == nullptr) {
                    writeWarning("Missing argument to option 'height-map'. Assuming no option.");
                    prog_opts.heightmap_input_file = "";
                } else {
                    prog_opts.heightmap_input_file = optarg;
                }
                break;
            case 4: // --no-skip-no-name-state
                prog_opts.no_skip_no_name_state = true;
                break;
            case 'v': // -v,--verbose
                if(prog_opts.quiet) {
                    writeError("Conflicting command line arguments 'v' and 'q'");
                    prog_opts.status = 1;
                    printHelp();
                    return prog_opts;
                }
                prog_opts.verbose = true;
                break;
            case 'q': // -q,--quiet
                if(prog_opts.verbose) {
                    writeError("Conflicting command line arguments 'v' and 'q'");
                    prog_opts.status = 1;
                    printHelp();
                    return prog_opts;
                }
                prog_opts.quiet = true;
                break;
            case 'h': // -h,--help
                prog_opts.status = 2;
                printHelp();
                return prog_opts;
            case '?':
                writeError("Unknown command line argument `"s + argv[optind - 1] + '`');
                prog_opts.status = 1;
                printHelp();
                return prog_opts;
            default:
                writeError("Unrecognized getopt_long code `"s + std::to_string(optopt) + '`');
                prog_opts.status = 1;
                printHelp();
                return prog_opts;
        }
    } while(c != -1);

    // Grab non-flag arguments
    if(auto i = optind; i < argc - 1) {
        prog_opts.infilename = argv[i];
        prog_opts.outpath = argv[i + 1];
    } else {
        writeError("Missing required argument(s)");
        prog_opts.status = 1;
        printHelp();
    }

    return prog_opts;
}



