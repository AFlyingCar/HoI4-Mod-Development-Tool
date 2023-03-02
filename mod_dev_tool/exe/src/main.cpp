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

#include <libintl.h>

#ifdef WIN32
# define WIN32_LEAN_AND_MEAN
# include <Windows.h>
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
#include "Preferences.h"
#include "PreprocessorUtils.h"

#include "Logger.h"
#include "ConsoleOutputFunctions.h"

#include "Interfaces.h"
#include "Logger.h"

namespace HMDT {
    ProgramOptions prog_opts;

    Preferences::SectionMap config_defaults =
PREF_BEGIN_DEF()
    // General program related settings
    PREF_BEGIN_DEFINE_SECTION(HMDT_LOCALIZE("General"), HMDT_LOCALIZE("General program related settings."))
        PREF_SECTION_DEFINE_PROPERTY(showTitles, true)

        PREF_BEGIN_DEFINE_GROUP(HMDT_LOCALIZE("Interface"), HMDT_LOCALIZE("Settings that control the interface of the program."))
            PREF_DEFINE_CONFIG(HMDT_LOCALIZE("language"), "en_US", HMDT_LOCALIZE("The language to be used."), true)
        PREF_END_DEFINE_GROUP()
    PREF_END_DEFINE_SECTION(),

    // Gui related settings
    PREF_BEGIN_DEFINE_SECTION(HMDT_LOCALIZE("Gui"), HMDT_LOCALIZE("Gui related settings."))
        PREF_SECTION_DEFINE_PROPERTY(showTitles, false)

        PREF_BEGIN_DEFINE_GROUP("_",)
            PREF_DEFINE_CONFIG(HMDT_LOCALIZE("darkMode"), false, HMDT_LOCALIZE("Whether the program should use dark-mode."), false)
        PREF_END_DEFINE_GROUP()
    PREF_END_DEFINE_SECTION(),

    // HoI4-info related settings
    PREF_BEGIN_DEFINE_SECTION(HMDT_LOCALIZE("HoI4"), HMDT_LOCALIZE("Settings related to interacting with Hearts of Iron 4."))
        PREF_SECTION_DEFINE_PROPERTY(showTitles, false)

        PREF_BEGIN_DEFINE_GROUP("_",)
            PREF_DEFINE_CONFIG(HMDT_LOCALIZE("installPath"), "", HMDT_LOCALIZE("The path of where Hearts of Iron 4 is installed."), false)
        PREF_END_DEFINE_GROUP()
    PREF_END_DEFINE_SECTION(),

    // Debug related settings
    PREF_BEGIN_DEFINE_SECTION(HMDT_LOCALIZE("Debug"), HMDT_LOCALIZE("Debug related settings."))
        PREF_SECTION_DEFINE_PROPERTY(showTitles, true)

        PREF_BEGIN_DEFINE_GROUP(HMDT_LOCALIZE("Logging"), HMDT_LOCALIZE("Logging settings."))
            PREF_DEFINE_CONFIG(HMDT_LOCALIZE("logPath"), "", HMDT_LOCALIZE("Overrides where the log files are written to."), true)
            PREF_DEFINE_CONFIG(HMDT_LOCALIZE("openLogWindowOnLaunch"), false, HMDT_LOCALIZE("Whether the log window should be opened on launch."), false)
        PREF_END_DEFINE_GROUP()

        PREF_BEGIN_DEFINE_GROUP(HMDT_LOCALIZE("Graphics"), HMDT_LOCALIZE("Graphical debug settings."))
            PREF_DEFINE_CONFIG(HMDT_LOCALIZE("renderAdjacenciesByDefault"), false, HMDT_LOCALIZE("Whether adjacent provinces should be rendered by default."), false)
        PREF_END_DEFINE_GROUP()
    PREF_END_DEFINE_SECTION()
PREF_END_DEF();

}

std::filesystem::path getAppLocalPath() {
    std::filesystem::path applocal_path;
#ifdef WIN32
    TCHAR path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path)))
    {
        applocal_path = path;
    }

    applocal_path = applocal_path / HMDT::APPLICATION_SIMPLE_NAME;
#else
    static struct passwd* user_pw = getpwuid(getuid());
    char* home_dir = user_pw->pw_dir;

    applocal_path = home_dir;
    applocal_path = applocal_path / ".local" / HMDT::APPLICATION_SIMPLE_NAME;
#endif

    if(!std::filesystem::exists(applocal_path)) {
        std::filesystem::create_directory(applocal_path);
    }

    return applocal_path;
}

/**
 * @brief Gets the path to the file where logs should get written to
 *
 * @return 
 */
std::filesystem::path getLogOutputFilePath() {
    return getAppLocalPath() / (HMDT::APPLICATION_SIMPLE_NAME + HMDT::LOG_FILE_EXTENSION);
}

std::filesystem::path getPreferencesPath() {
    return getAppLocalPath() / (HMDT::APPLICATION_SIMPLE_NAME + HMDT::CONF_FILE_EXTENSION);
}

/**
 * @brief Initializes and loads the preferences file
 *
 * @return True on success, false otherwise
 */
auto initializePreferences() -> HMDT::MaybeVoid {
    // First, lets initialize the Preferences with the default values
    HMDT::Preferences::getInstance(false).setDefaultValues(HMDT::config_defaults);

    // We have to make sure we also call this here so that the values exist so
    //   the callbacks can be registered
    HMDT::Preferences::getInstance(false).resetToDefaults();

    // Set up all preference change callbacks

    // If the language gets changed, make sure that we update the locale
    //   internally as well
    auto result = HMDT::Preferences::getInstance(false)
        .setCallbackOnPreferenceChange("General.Interface.language",
            [](const auto& old_value, const auto& new_value) -> bool {
                // new_value should always hold a std::string
                auto locale = std::get<std::string>(new_value);

                // TODO: Validate that the locale is a valid one that we support

                // Code adapted from: https://erri120.github.io/posts/2022-05-05/
                WRITE_DEBUG("Updating locale from ",
                            std::get<std::string>(old_value), " to ", locale);
#if WIN32
                // LocaleNameToLCID requires a LPCWSTR so we need to convert from char to wchar_t
                const auto wStringSize = MultiByteToWideChar(CP_UTF8, 0, locale.c_str(), static_cast<int>(locale.length()), nullptr, 0);
                std::wstring localeName;
                localeName.reserve(wStringSize);
                MultiByteToWideChar(CP_UTF8, 0, locale.c_str(), static_cast<int>(locale.length()), localeName.data(), wStringSize);

                _configthreadlocale(_DISABLE_PER_THREAD_LOCALE);
                const auto localeId = LocaleNameToLCID(localeName.c_str(), LOCALE_ALLOW_NEUTRAL_NAMES);
                SetThreadLocale(localeId);
#else
                setlocale(LC_MESSAGES, locale.c_str());

                // Make sure the LANGUAGE envvar is set
                setenv("LANGUAGE", locale.c_str(), 1 /* overwrite */);
#endif

                return true;
            });
    RETURN_IF_ERROR(result);

    // Do this again after all callbacks have been registered so that we can
    //   make sure that they all get called at least once
    HMDT::Preferences::getInstance(false).resetToDefaults();

    // TODO: Allow overwriting by cmd argument
    auto pref_path = getPreferencesPath();

    // Now set the Preferences path
    HMDT::Preferences::getInstance(false).setConfigLocation(pref_path);

    // Next, does the preferences file actually exist on the disk?
    //   If not, then lets create it
    if(!std::filesystem::exists(pref_path)) {
        WRITE_INFO("Config file at ", pref_path,
                   " does not exist, going to use the default values, and write"
                   " a default file to the disk at that path.");

        // But don't bother creating it if the directory it should go into does
        //   not exist
        if(!std::filesystem::exists(pref_path.parent_path())) {
            WRITE_ERROR("Cannot write preferences to ", pref_path,
                        ": Directory '", pref_path.parent_path(),
                        "' does not exist.");
            RETURN_ERROR(std::make_error_code(std::errc::no_such_file_or_directory));
        }

        // Write the preferences to a file, and from here we will use the
        //   default values
        result = HMDT::Preferences::getInstance().writeToFile(true /* pretty */);
        RETURN_IF_ERROR(result);
    } else {
        // If it does exist on the disk, load and validate it
        result = HMDT::Preferences::getInstance().validateLoadedPreferenceTypes();
        RETURN_IF_ERROR(result);
    }

    return HMDT::STATUS_SUCCESS;
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
    // Set up text domains
    bindtextdomain(HMDT_TOOL_NAME, (HMDT::getExecutablePath() / "locale").c_str());
    bind_textdomain_codeset(HMDT_TOOL_NAME, "UTF-8");
    textdomain(HMDT_TOOL_NAME);

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
            if(!is_verbose && level == HMDT::Log::Message::Level::DEBUG) {
                return true;
            }

            // Info and Debug only output if quiet is false
            // No need to check for debug because quiet and verbose cannot both
            //  be true at the same time
            if(is_quiet && level == HMDT::Log::Message::Level::INFO) {
                return true;
            }

            return HMDT::Log::outputWithFormatting(message);
        });

    // Set up a user-data pointer that will be registered with the output
    //   function
    using UDType = std::tuple<std::ofstream, std::queue<HMDT::Log::Message>>;
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

    WRITE_DEBUG("Searching for localization files in ",
                HMDT::getExecutablePath() / "locale");

    // Parse the command-line arguments
    HMDT::prog_opts = HMDT::parseArgs(argc, argv);

    auto result = initializePreferences();
    if(IS_FAILURE(result)) {
        WRITE_ERROR("Failed to initialize preferences! Exiting...");
        return 1;
    }

    if(!HMDT::prog_opts.dont_write_logfiles) {
        auto log_output_path = getLogOutputFilePath();

        // Overwrite the log path if it is specified in the log path
        HMDT::Preferences::getInstance().getPreferenceValue<std::string>("Debug.Logging.logPath")
            .andThen([&log_output_path](const std::string& log_path) {
                if(!log_path.empty()) {
                    WRITE_INFO("Overwriting log path to be '", log_path);
                    log_output_path = log_path;
                }
            });

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

