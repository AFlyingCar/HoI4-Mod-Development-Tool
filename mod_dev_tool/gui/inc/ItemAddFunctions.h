#ifndef ITEM_ADD_FUNCTIONS_H
# define ITEM_ADD_FUNCTIONS_H

# include <any>
# include <vector>
# include <filesystem>

# include "Maybe.h"

# include "Window.h"

namespace HMDT::GUI {
    Maybe<std::any> initAddProvinceMap(Window&,
                                       const std::vector<std::filesystem::path>&);
    MaybeVoid addProvinceMapWorker(Window&, std::any);
    MaybeVoid postStartAddProvinceMap(Window&, std::any);
    MaybeVoid endAddProvinceMap(Window&, std::any);
}

#endif

