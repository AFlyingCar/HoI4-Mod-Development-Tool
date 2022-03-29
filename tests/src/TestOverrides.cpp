/**
 * @brief Implements classes, variables, and functions that are only found in
 *        exe and gui, which are depended on by code that is getting tested, but
 *        for which we don't want to or can't link into the test executable
 *        itself
 */

#include "TestOverrides.h"

MapNormalizer::ProgramOptions MapNormalizer::prog_opts = {
    0, "", "", false, false, "", "", false, "", false, false, false, false, false
};

