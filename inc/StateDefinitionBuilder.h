
#ifndef STATE_DEFINITION_BUILDER_H
#define STATE_DEFINITION_BUILDER_H

#include <filesystem>

#include "Types.h"

namespace MapNormalizer {
    StateList createStatesList(const ProvinceList&,
                               const std::filesystem::path& = "");
}

#endif

