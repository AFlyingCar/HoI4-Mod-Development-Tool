
#include "TestUtils.h"

#include <cstdlib>

#ifdef _WIN32
# include "windows.h"
# ifndef PATH_MAX
#  define PATH_MAX FILENAME_MAX
# endif
#else
# include <unistd.h>
# include <linux/limits.h>
#endif

#include "Logger.h"
#include "Message.h"
#include "ConsoleOutputFunctions.h"

#define ENVVAR_PREFIX ENVVAR_

#define DEF_ENVVAR_IMPL(VARNAME) \
    const char* const VARNAME = STR(VARNAME)

#define DEF_ENVVAR(VARNAME) \
    DEF_ENVVAR_IMPL(CONCAT(ENVVAR_PREFIX, VARNAME))

DEF_ENVVAR(VERBOSE);

HMDT::UnitTests::NullStream cnul;

bool HMDT::UnitTests::useVerboseOutput() {
    return std::getenv(ENVVAR_VERBOSE) != nullptr;
}

std::filesystem::path HMDT::UnitTests::getTestProgramPath() {
    char path[PATH_MAX] = { 0 };
#ifdef _WIN32
    GetModuleFileName(NULL, path, PATH_MAX);
#else
    readlink("/proc/self/exe", path, PATH_MAX);
#endif

    return std::filesystem::path(path).parent_path();
}

/**
 * @brief Registers a log output function to stdout.
 *
 * @param include_formatting Whether formatting should be included.
 * @param include_prefix Whether the prefix should be included.
 * @param include_timestamp Whether the timestamp should be included.
 * @param include_src_info Whether source information should be included.
 */
void HMDT::UnitTests::registerTestLogOutputFunction(bool include_formatting,
                                                    bool include_prefix,
                                                    bool include_timestamp,
                                                    bool include_src_info)
{
    Log::Logger::registerOutputFunction([=](const Log::Message& m)
    {
        return Log::outputToStream(m, include_formatting, include_prefix,
            [](uint8_t debug_level) -> std::ostream& {
                switch(static_cast<Log::Message::Level>(debug_level)) {
                    case Log::Message::Level::INFO:
                        return TEST_COUT;
                    case Log::Message::Level::ERROR:
                    case Log::Message::Level::WARN:
                        return TEST_CERR;
                    case Log::Message::Level::DEBUG:
                        // If verbose output is enabled, then output debug
                        //  otherwise send it to cnul as well.
                        if(useVerboseOutput()) {
                            return TEST_COUT;
                        }
                    [[fallthrough]];
                    default:
                        return cnul;
                }
            }, include_timestamp, include_src_info);
    });
}

