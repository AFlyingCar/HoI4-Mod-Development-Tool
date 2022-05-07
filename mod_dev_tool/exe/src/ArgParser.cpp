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
static std::string program_name = "hoi4_mod_dev_tool";

/**
 * @brief Prints help information to the console.
 */
void HMDT::printHelp() {
    std::cout << program_name << " [OPTIONS...] {[INFILE] [OUTPATH]}";
    std::cout << "\t   --no-gui                Alias for --headless.";
    std::cout << "\t   --no-skip-no-name-state Do not skip states with no name.";
    std::cout << "\t   --state-input           The input file for writing state definitions.";
    std::cout << "\t   --height-map            The input file for writing the normal map.";
    std::cout << "\t   --hoi4-install-path     The installation path for Hearts of Iron 4.";
    std::cout << "\t   --output-stages         Output every stage of the shape detection algorithm.";
    std::cout << "\t   --headless              Should the application run in headless mode (without the GUI). Note that this requires [INFIILE] and [OUTPATH] to be provided.";
    std::cout << "\t   --debug                 Should debugging features be enabled.";
    std::cout << "\t   --dont-write-logfiles   Should log files get written to a file.";
    std::cout << "\t   --fix-warnings-on-load  Whether or not problems in a project file should attempt to be fixed when they are loaded.";
    std::cout << "\t-v,--verbose               Display all output.";
    std::cout << "\t-q,--quiet                 Display only errors and warnings (does not affect this message).";
    std::cout << "\t-h,--help                  Display this message and exit.";
}

/**
 * @brief Parses command-line arguments.
 *
 * @param argc The number of arguments.
 * @param argv The array of arguments.
 *
 * @return A structure containing global program options.
 */
auto HMDT::parseArgs(int argc, char** argv) -> ProgramOptions {
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
        { "hoi4-install-path", required_argument, NULL, 5 },
        { "output-stages", no_argument, NULL, 6 },
        { "headless", no_argument, NULL, 7 },
        { "debug", no_argument, NULL, 8 },
        { "dont-write-logfiles", no_argument, NULL, 9 },
        { "fix-warnings-on-load", no_argument, NULL, 10 },
        { nullptr, 0, nullptr, 0}
    };

    // Setup default option values
    ProgramOptions prog_opts { 0, "", "", false, false, "", "", false, "", false, false, false, false, false };

    int optindex = 0;
    int c = 0;

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
                WRITE_DEBUG("opterr = "s + std::to_string(opterr));
                WRITE_DEBUG("optopt = "s + std::to_string(optopt));
                WRITE_DEBUG("optind = "s + std::to_string(optind));
                if(optarg)
                    WRITE_DEBUG("optarg = "s + optarg);
                else
                    WRITE_DEBUG("optarg = 0x0");
                break;
            case 2: // --state-input
                if(optarg == nullptr) {
                    WRITE_WARN("Missing argument to option 'state-input'. Assuming no option.");
                    prog_opts.state_input_file = "";
                } else {
                    prog_opts.state_input_file = optarg;
                }
                break;
            case 3: // --height-map
                if(optarg == nullptr) {
                    WRITE_WARN("Missing argument to option 'height-map'. Assuming no option.");
                    prog_opts.heightmap_input_file = "";
                } else {
                    prog_opts.heightmap_input_file = optarg;
                }
                break;
            case 4: // --no-skip-no-name-state
                prog_opts.no_skip_no_name_state = true;
                break;
            case 5: // --hoi4-install-path
                if(optarg == nullptr) {
                    WRITE_WARN("Missing argument to option 'hoi4-install-path'. Assuming no option.");
                    prog_opts.hoi4_install_path = "";
                } else {
                    prog_opts.hoi4_install_path = optarg;
                }
                break;
            case 6: // --output-stages
                prog_opts.output_stages = true;
                break;
            case 1: // --no-gui
            case 7: // --headless
                prog_opts.headless = true;
                break;
            case 8: // --debug
                prog_opts.debug = true;
                break;
            case 9: // --dont-write-logfiles
                prog_opts.dont_write_logfiles = true;
                break;
            case 10: // --fix-warnings-on-load
                prog_opts.fix_warnings_on_load = true;
                break;
            case 'v': // -v,--verbose
                if(prog_opts.quiet) {
                    WRITE_ERROR("Conflicting command line arguments 'v' and 'q'");
                    prog_opts.status = 1;
                    printHelp();
                    return prog_opts;
                }
                prog_opts.verbose = true;
                break;
            case 'q': // -q,--quiet
                if(prog_opts.verbose) {
                    WRITE_ERROR("Conflicting command line arguments 'v' and 'q'");
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
                WRITE_ERROR("Unknown command line argument `"s + argv[optind - 1] + '`');
                prog_opts.status = 1;
                printHelp();
                return prog_opts;
            default:
                WRITE_ERROR("Unrecognized getopt_long code `"s + std::to_string(optopt) + '`');
                prog_opts.status = 1;
                printHelp();
                return prog_opts;
        }
    } while(c != -1);

    // Grab non-flag arguments
    if(auto i = optind; i < argc - 1) {
        prog_opts.infilename = argv[i];
        prog_opts.outpath = argv[i + 1];
    } else if(prog_opts.headless) {
        // We only require the file options if we are in headless mode
        WRITE_ERROR("Missing required argument(s)");
        prog_opts.status = 1;
        printHelp();
    }

    return prog_opts;
}



