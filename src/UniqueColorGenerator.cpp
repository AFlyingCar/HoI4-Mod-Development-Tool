
#include "UniqueColorGenerator.h"
#include "Constants.h"
#include "ColorArray.h"

#define NOMINMAX
#include <cmath>
#include <algorithm>

#include <iostream>

// Note: if we are completely out of color values for some bias, then we will give back BLACK
//   If we don't know what the bias is, then we return WHITE
MapNormalizer::Color MapNormalizer::generateUniqueColor(ProvinceType bias) {
    static const unsigned char* land_ptr = MN_ALL_LANDS;
    static const unsigned char* sea_ptr = MN_ALL_SEAS;
    static const unsigned char* lake_ptr = MN_ALL_LAKES;
    static const unsigned char* unknown_ptr = MN_ALL_UNKNOWNS;

    bool err = false;
    Color c;

    if(land_ptr >= MN_ALL_LANDS + MN_ALL_LANDS_SIZE) {
        std::cerr << "[WRN] ~ NO LAND VALUES LEFT!" << std::endl;
        err = true;
        goto unknown_color_label;
    }

    if(sea_ptr >= MN_ALL_SEAS + MN_ALL_SEAS_SIZE) {
        std::cerr << "[WRN] ~ NO SEA VALUES LEFT!" << std::endl;
        err = true;
        goto unknown_color_label;
    }

    if(lake_ptr >= MN_ALL_LAKES + MN_ALL_LAKES_SIZE) {
        std::cerr << "[WRN] ~ NO LAKE VALUES LEFT!" << std::endl;
        err = true;
        goto unknown_color_label;
    }

    switch(bias) {
        case ProvinceType::LAND:
            c = Color { land_ptr[0], land_ptr[1], land_ptr[2] };
            land_ptr += 3;
            break;
        case ProvinceType::SEA:
            c = Color { lake_ptr[0], lake_ptr[1], lake_ptr[2] };
            lake_ptr += 3;
            break;
        case ProvinceType::LAKE:
            c = Color { sea_ptr[0], sea_ptr[1], sea_ptr[2] };
            sea_ptr += 3;
            break;
        default:
            err = true;
    }

unknown_color_label:
    if(err) {
        if(unknown_ptr >= MN_ALL_UNKNOWNS + MN_ALL_UNKNOWNS_SIZE) {
            std::cerr << "[ERR] ~ NO UNKNOWN COLOR VALUES LEFT!";
            return Color { 0, 0, 0 }; // Last possible resort
        }

        c = Color { unknown_ptr[0], unknown_ptr[1], unknown_ptr[2] };
        unknown_ptr += 3;
    }

    return c;
}

