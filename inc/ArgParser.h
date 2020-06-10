/**
 * @file ArgParser.h
 *
 * @brief Defines the structure for storing global program options, and the
 *        functions for parsing command-line arguments.
 */

#ifndef ARG_PARSER_H
# define ARG_PARSER_H

# include <string>

namespace MapNormalizer {
    /**
     * @brief The structure for storing global program options.
     */
    struct ProgramOptions {
        //! The status code for if the arguments were correctly parsed.
        int status;

        //! The input filename
        std::string infilename;

        //! The output filename
        std::string outpath;

        //! -v,--verbose
        bool verbose;

        //! -q,--quiet
        bool quiet;

        //! --no-gui
        bool no_gui;

        //! --state-input=
        std::string state_input_file;

        //! --height-map=
        std::string heightmap_input_file;

        //! --no-skip-no-name-state
        bool no_skip_no_name_state;

        //! --hoi4-install-path=
        std::string hoi4_install_path;
    };

    ProgramOptions parseArgs(int, char**);

    void printHelp();
}

#endif

