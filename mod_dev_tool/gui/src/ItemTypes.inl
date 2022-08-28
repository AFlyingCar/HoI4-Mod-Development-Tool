/**
 * @file ItemTypes.inl
 * 
 * @brief Define every item type here. Make sure that every item type name is
 *        unique.
 */

DEFINE_ITEM_TYPE(
    "Province Map",
    "Province Maps",
    "",
    {
        "map/*",
        "history/states/*"
    },
    HMDT::GUI::initAddProvinceMap,
    HMDT::GUI::addProvinceMapWorker,
    HMDT::GUI::postStartAddProvinceMap,
    HMDT::GUI::endAddProvinceMap,
    [](HMDT::GUI::Window& parent_window) -> HMDT::MaybeVoid {
        return HMDT::STATUS_NOT_IMPLEMENTED;
    }
);

