/**
 * @file Options.h
 *
 * @brief Defines the global prog_opts variable
 */

#ifndef OPTIONS_H
# define OPTIONS_H

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

        //! --output-stages
        bool output_stages;
    };

    //! Global variable for storing program options.
    extern ProgramOptions prog_opts;
}

#endif

