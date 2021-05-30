
#include "TestUtils.h"

#include <cstdlib>

#define ENVVAR_PREFIX ENVVAR_

#define DEF_ENVVAR_IMPL(VARNAME) \
    const char* const VARNAME = STR(VARNAME)

#define DEF_ENVVAR(VARNAME) \
    DEF_ENVVAR_IMPL(CONCAT(ENVVAR_PREFIX, VARNAME))

DEF_ENVVAR(VERBOSE);

bool MapNormalizer::UnitTests::useVerboseOutput() {
    return std::getenv(ENVVAR_VERBOSE) != nullptr;
}

