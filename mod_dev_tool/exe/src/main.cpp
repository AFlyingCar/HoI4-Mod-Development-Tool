/**
 * @file main.cpp
 *
 * @brief The starting point of the program
 */

#include <iostream>
#include <filesystem>
#include <fstream>
#include <queue>
#include <cerrno> // errno
#include <cstring> // strerror

#ifdef WIN32
# include <shlwapi.h>
# include "shlobj.h"
# ifdef ERROR
#  undef ERROR // This is necessary because we define ERROR in an enum, but windows defines it as something else
# endif
#else
# include <pwd.h>
# include <unistd.h>
#endif

#include "ArgParser.h"
#include "Options.h"
#include "Constants.h"

#include "Logger.h"
#include "ConsoleOutputFunctions.h"

#include "Interfaces.h"
#include "Logger.h"

HMDT::ProgramOptions HMDT::prog_opts;

/**
 * @brief Gets the path to the file where logs should get written to
 *
 * @return 
 */
std::filesystem::path getLogOutputFilePath() {
    std::filesystem::path log_path;
#ifdef WIN32
    TCHAR path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
    {
        log_path = path;
    }

    log_path = log_path / HMDT::APPLICATION_SIMPLE_NAME;
#else
    static struct passwd* user_pw = getpwuid(getuid());
    char* home_dir = user_pw->pw_dir;

    log_path = home_dir;
    log_path = log_path / ".local" / HMDT::APPLICATION_SIMPLE_NAME;
#endif

    if(!std::filesystem::exists(log_path)) {
        std::filesystem::create_directory(log_path);
    }

    log_path = log_path / (HMDT::APPLICATION_SIMPLE_NAME + HMDT::LOG_FILE_EXTENSION);

    return log_path;
}

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
    std::shared_ptr<bool> quiet(new bool(false));
    std::shared_ptr<bool> verbose(new bool(false));
    HMDT::Log::Logger::registerOutputFunction(
        [quiet, verbose](const HMDT::Log::Message& message) -> bool {
            bool is_quiet = quiet != nullptr && *quiet;
            bool is_verbose = verbose != nullptr && *verbose;

            const auto& level = message.getDebugLevel();

            // Debug only outputs if verbose is true
            if(!is_verbose && level == HMDT::Log::Message::Level::DEBUG)
            {
                return true;
            }

            // Info and Debug only output if quiet is false
            // No need to check for debug because quiet and verbose cannot both
            //  be true at the same time
            if(is_quiet && level == HMDT::Log::Message::Level::INFO)
            {
                return true;
            }

            return HMDT::Log::outputWithFormatting(message);
        });

    // Set up a user-data pointer that will be registered with the output
    //   function
    using UDType = std::tuple<std::ofstream,
                               std::queue<HMDT::Log::Message>>;
    std::shared_ptr<UDType> file_ud(new UDType);

    // Simple reference to the first part of the user-data pointer, as we will
    //  need to set this up later
    std::ofstream& log_output_file = std::get<0>(*file_ud);

    std::shared_ptr<bool> disable_file_log_output(new bool(false));

    HMDT::Log::Logger::registerOutputFunction(
        [disable_file_log_output](const HMDT::Log::Message& message,
                                  HMDT::Log::Logger::UserData user_data)

            -> bool
        {
            if(*disable_file_log_output) return true;

            std::shared_ptr<UDType> file_ud = std::static_pointer_cast<UDType>(user_data);
            auto& log_output_file = std::get<0>(*file_ud);
            auto& messages = std::get<1>(*file_ud);

            messages.push(message);

            if(!log_output_file) return true;

            while(!messages.empty()) {
                auto&& message = messages.front();
                if(!HMDT::Log::outputToStream(message, false, true, 
                    [&log_output_file](uint8_t) -> std::ostream& {
                        return log_output_file;
                    }, true))
                {
                    return false;
                }
                messages.pop();
            }

            return true;
        }, file_ud
    );

    // Parse the command-line arguments
    HMDT::prog_opts = HMDT::parseArgs(argc, argv);

    if(!HMDT::prog_opts.dont_write_logfiles) {
        auto log_output_path = getLogOutputFilePath();
        log_output_file.open(log_output_path);

        if(!log_output_file) {
            WRITE_ERROR("Failed to open ", log_output_path, ". Reason: ", strerror(errno));
            *disable_file_log_output = true;
        } else {
            WRITE_INFO("Log files will get written to ", log_output_path);
        }
    } else {
        *disable_file_log_output = true;
    }

    // Figure out if we should stop now based on the status of the parsing
    switch(HMDT::prog_opts.status) {
        case 1:
            WRITE_ERROR("Failed to parse program options. Exiting now.");
            return 1;
        case 2:
            return 0;
        case 0:
        default:
            break;
    }

    *quiet = HMDT::prog_opts.quiet;
    *verbose = HMDT::prog_opts.verbose;

    try {
        return HMDT::runApplication();
    } catch(const std::exception& e) {
        WRITE_ERROR(e.what());
        return -1;
    }
}

