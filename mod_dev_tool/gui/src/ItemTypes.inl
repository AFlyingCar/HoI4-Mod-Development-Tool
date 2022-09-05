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
    "Province maps are 24-bit RGB bitmaps which describe the positions of every"
    " province in a custom map. It is recommended that the dimensions be a "
    "multiple of 256 each, as while this tool can load files which do not match"
    " those dimensions, the HoI4 engine cannot. Additionally, HoI4 does not "
    "allow province maps with a pixel area exceeding 13'107'200.\n\n"
    "When loading these maps in this tool, we will detect each province in the "
    "input image, and only require that they be all separated by a boundary of "
    "pixels made of the RGB color value (0,0,0). Additionally, for automated "
    "detection of province types (LAND, SEA, LAKE), you can color in the "
    "provinces with a solid color to denote which one is which. This will allow"
    " the tool to fill in this information about each province automatically, "
    "choose unique random colors from appropriate color pools (all "
    "automatically chosen SEA tiles will get blue-ish colors, for example), "
    "as well as determine other information automatically, such as whether a "
    "given province is a coastal tile or not.\n\n"
    "For additional information, see the example project on the github." /* description */,
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

