
#include "UniqueColorGenerator.h"
#include "Constants.h"
#include "ColorArray.h"
#include "Logger.h"

#define NOMINMAX
#include <cmath>
#include <algorithm>

#include <iostream> // std::cerr

/**
 * @brief Generates a unique color value.
 * @details If there are no more color values for the given bias, then give out
 *          BLACK instead. If we don't recognize the bias, then give out WHITE
 *          instead.
 *
 * @param bias The type of province which will change the type of color chosen.
 * @return A unique color, biased based on the given ProvinceType. Will return
 *         BLACK if no color values are left, or WHITE if the given bias is
 *         unknown.
 */
MapNormalizer::Color MapNormalizer::generateUniqueColor(ProvinceType bias) {
    static const unsigned char* land_ptr = MN_ALL_LANDS;
    static const unsigned char* sea_ptr = MN_ALL_SEAS;
    static const unsigned char* lake_ptr = MN_ALL_LAKES;
    static const unsigned char* unknown_ptr = MN_ALL_UNKNOWNS;

    bool err = false;
    Color c;

    if(land_ptr >= MN_ALL_LANDS + MN_ALL_LANDS_SIZE) {
        writeWarning("NO LAND VALUES LEFT!");
        err = true;
        goto unknown_color_label;
    }

    if(sea_ptr >= MN_ALL_SEAS + MN_ALL_SEAS_SIZE) {
        writeWarning("NO SEA VALUES LEFT!");
        err = true;
        goto unknown_color_label;
    }

    if(lake_ptr >= MN_ALL_LAKES + MN_ALL_LAKES_SIZE) {
        writeWarning("NO LAKE VALUES LEFT!");
        err = true;
        goto unknown_color_label;
    }

    switch(bias) {
        case ProvinceType::LAND:
            c = Color { land_ptr[0], land_ptr[1], land_ptr[2] };
            land_ptr += 3;
            break;
        case ProvinceType::LAKE:
            c = Color { lake_ptr[0], lake_ptr[1], lake_ptr[2] };
            lake_ptr += 3;
            break;
        case ProvinceType::SEA:
            c = Color { sea_ptr[0], sea_ptr[1], sea_ptr[2] };
            sea_ptr += 3;
            break;
        default:
            err = true;
    }

unknown_color_label:
    if(err) {
        if(unknown_ptr >= MN_ALL_UNKNOWNS + MN_ALL_UNKNOWNS_SIZE) {
            writeError("NO UNKNOWN COLOR VALUES LEFT!");
            return Color { 0, 0, 0 }; // Last possible resort
        }

        c = Color { unknown_ptr[0], unknown_ptr[1], unknown_ptr[2] };
        unknown_ptr += 3;
    }

    return c;
}

