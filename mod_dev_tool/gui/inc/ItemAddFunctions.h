#ifndef ITEM_ADD_FUNCTIONS_H
# define ITEM_ADD_FUNCTIONS_H

# include <any>
# include <filesystem>

# include "Maybe.h"

# include "Window.h"

namespace HMDT::GUI {
    Maybe<std::any> initAddProvinceMap(Window&, const std::filesystem::path&);
    MaybeVoid startAddProvinceMap(Window&, std::any);
    Maybe<bool> addProvinceMap(Window&, std::any);
    MaybeVoid endAddProvinceMap(Window&, std::any);
}

#endif

