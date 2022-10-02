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
    "Province Map Input" /* name */,
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

DEFINE_ITEM_TYPE(
    "Height Map" /* name */,
    "Heightmaps are 8-bit greyscale bitmaps which determine the height of the "
    "positions on the map. The dimensions of this bitmap <i>must</i> match the "
    "dimensions of the provinces image.\n\n"
    "A minimum value of 0 (pure black) will translate to a height of 0 on the "
    "Y-axis, while a maximum value of 255 (pure white) will translate to a "
    "height of 25.5 on the Y-axis. HoI4 renders the ocean at a height of 9.5 "
    "by default (this can be modified however, see the HoI4 wiki about "
    "heightmaps for more information).\n\n"
    "Finally, note that this item will also automatically generate a world "
    "normal map when exporting the project."/* description */,
    "/com/aflyingcar/HoI4ModDevelopmentTool/textures/heightmap.png" /* icon */,
    {{
        { { "Height Map Image Files", "bmp" } } /* filters */,
        false /* allow_multiselect */,
    }} /* file_info */,
    {
        "map/*",
        "history/states/*"
    } /* extra_overrides */,
    HMDT::GUI::addHeightMap /* init_add_callback */,
    HMDT::GUI::ItemType::DEFAULT_POSTINIT_CALLBACK /* add_worker_callback */,
    HMDT::GUI::ItemType::DEFAULT_POSTINIT_CALLBACK /* post_start_add_callback */,
    HMDT::GUI::ItemType::DEFAULT_POSTINIT_CALLBACK /* end_add_callback  */,
    [](HMDT::GUI::Window& parent_window) -> HMDT::MaybeVoid {
        return HMDT::STATUS_NOT_IMPLEMENTED;
    } /* on_remove_callback */
);

DEFINE_ITEM_TYPE(
    "River Map" /* name */,
    "Rivers is an 8-bit indexed bitmap file which determines the positioning "
    "of rivers. The dimensions of this bitmap <i>must</i> match the dimensions "
    "of the provinces image.\n\n"
    "Rivers <i>must</i> be exactly one pixel thick and only go in orthogonal "
    "directions (they cannot connect diagonally). To render correctly, each "
    "river must have exactly <i>one</i> start marker (green by default), where "
    "each river is defined as a single contiguous block of river pixels. "
    "Pixels connected with flow-in (red by default) or flow-out (yellow by "
    "default) sources are counted as the same river as the main flow. Only the "
    "main branch of the river should have a start marker.\n\n"
    "For the specific indices + color values, see the HoI4 modding wiki." /* description */,
    "/com/aflyingcar/HoI4ModDevelopmentTool/textures/rivermap.png" /* icon */,
    {{
        { { "River Map Image Files", "bmp" } } /* filters */,
        false /* allow_multiselect */,
    }} /* file_info */,
    {
        "map/*",
        "history/states/*"
    } /* extra_overrides */,
    HMDT::GUI::addRivers /* init_add_callback */,
    HMDT::GUI::ItemType::DEFAULT_POSTINIT_CALLBACK /* add_worker_callback */,
    HMDT::GUI::ItemType::DEFAULT_POSTINIT_CALLBACK /* post_start_add_callback */,
    HMDT::GUI::ItemType::DEFAULT_POSTINIT_CALLBACK /* end_add_callback  */,
    [](HMDT::GUI::Window& parent_window) -> HMDT::MaybeVoid {
        return HMDT::STATUS_NOT_IMPLEMENTED;
    } /* on_remove_callback */
);

