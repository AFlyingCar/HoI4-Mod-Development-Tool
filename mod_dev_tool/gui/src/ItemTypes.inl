/**
 * @file ItemTypes.inl
 * 
 * @brief Define every item type here. Make sure that every item type name is
 *        unique.
 */

DEFINE_ITEM_TYPE(
    "Province Map" /* name */,
    "Province Maps" /* description */,
    "" /* icon */,
    {
        "map/*",
        "history/states/*"
    } /* extra_overrides */,
    HMDT::GUI::initAddProvinceMap /* init_add_callback */,
    HMDT::GUI::addProvinceMapWorker /* add_worker_callback */,
    HMDT::GUI::postStartAddProvinceMap /* post_start_add_callback */,
    HMDT::GUI::endAddProvinceMap /* end_add_callback */,
    [](HMDT::GUI::Window& parent_window) -> HMDT::MaybeVoid {
        return HMDT::STATUS_NOT_IMPLEMENTED;
    } /* on_remove_callback */
);

