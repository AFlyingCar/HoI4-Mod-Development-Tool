/**
 * @file ItemTypes.inl
 * 
 * @brief Define every item type here.
 *
 * @details Make sure that every item type name is unique, clashes will override
 *          earlier definitions.
 */

/////////////////////////////////////////////////
// DO NOT WRITE ANY #include STATEMENTS HERE!  //
//                                             //
// Any includes should go in ItemRegistrar.cpp //
/////////////////////////////////////////////////

DEFINE_ITEM_TYPE(
    "Province Map" /* name */,
    "Description of how Province Maps work. TODO: Finish writing this." /* description */,
    "/com/aflyingcar/HoI4ModDevelopmentTool/textures/provinces.png" /* icon */,
    {{
        { { "Province Image Files", "bmp" } } /* filters */,
        false /* allow_multiselect */,
    }} /* file_info */,
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

