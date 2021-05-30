
#include "TestUtils.h"

#include <cstdlib>

#ifdef _WIN32
# include "windows.h"
# define PATH_MAX FILENAME_MAX
#else
# include <unistd.h>
# include <linux/limits.h>
#endif

#define ENVVAR_PREFIX ENVVAR_

#define DEF_ENVVAR_IMPL(VARNAME) \
    const char* const VARNAME = STR(VARNAME)

#define DEF_ENVVAR(VARNAME) \
    DEF_ENVVAR_IMPL(CONCAT(ENVVAR_PREFIX, VARNAME))

DEF_ENVVAR(VERBOSE);

bool MapNormalizer::UnitTests::useVerboseOutput() {
    return std::getenv(ENVVAR_VERBOSE) != nullptr;
}

std::filesystem::path MapNormalizer::UnitTests::getTestProgramPath() {
    char path[PATH_MAX];
#ifdef _WIN32
    GetModuleFileName(NULL, path, PATH_MAX);
#else
    readlink("/proc/self/exe", path, PATH_MAX);
#endif

    return std::filesystem::path(path).parent_path();
}

