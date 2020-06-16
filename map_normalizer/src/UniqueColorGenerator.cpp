
#include "UniqueColorGenerator.h"
#include "ColorArray.h"
#include "Logger.h"

#define NOMINMAX
#include <cmath>
#include <algorithm>

#include <iostream> // std::cerr

namespace {
    using UniqueColorPtr = const unsigned char*;

    UniqueColorPtr getUniqueColorPtrStart(MapNormalizer::ProvinceType bias) {
        switch(bias) {
            case MapNormalizer::ProvinceType::LAND:
                return MN_ALL_LANDS;
            case MapNormalizer::ProvinceType::SEA:
                return MN_ALL_SEAS;
            case MapNormalizer::ProvinceType::LAKE:
                return MN_ALL_LAKES;
            default:
                return MN_ALL_UNKNOWNS;
        }
    }

    unsigned int getUniqueColorPtrSize(MapNormalizer::ProvinceType bias) {
        switch(bias) {
            case MapNormalizer::ProvinceType::LAND:
                return MN_ALL_LANDS_SIZE;
            case MapNormalizer::ProvinceType::SEA:
                return MN_ALL_SEAS_SIZE;
            case MapNormalizer::ProvinceType::LAKE:
                return MN_ALL_LAKES_SIZE;
            default:
                return MN_ALL_UNKNOWNS_SIZE;
        }
    }

    UniqueColorPtr& getUniqueColorPtr(MapNormalizer::ProvinceType bias) {
        static const unsigned char* land_ptr = MN_ALL_LANDS;
        static const unsigned char* sea_ptr = MN_ALL_SEAS;
        static const unsigned char* lake_ptr = MN_ALL_LAKES;
        static const unsigned char* unknown_ptr = MN_ALL_UNKNOWNS;

        switch(bias) {
            case MapNormalizer::ProvinceType::LAND:
                return land_ptr;
            case MapNormalizer::ProvinceType::SEA:
                return sea_ptr;
            case MapNormalizer::ProvinceType::LAKE:
                return lake_ptr;
            default:
                return unknown_ptr;
        }
    }

    bool verifyColorsRemain(MapNormalizer::ProvinceType bias) {
        UniqueColorPtr color_ptr = getUniqueColorPtr(bias);
        UniqueColorPtr start_ptr = getUniqueColorPtrStart(bias);
        unsigned int size = getUniqueColorPtrSize(bias);

        if(color_ptr >= start_ptr + size) {
            MapNormalizer::writeWarning("NO VALUES LEFT!");
            return false;
        }

        return true;
    }
}

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
    UniqueColorPtr& color_ptr = getUniqueColorPtr(bias);

    bool err = (bias == ProvinceType::UNKNOWN);
    Color c;

    if(err) {
        goto unknown_color_label;
    }

    if(!verifyColorsRemain(bias)) {
        err = true;
        goto unknown_color_label;
    }

    c = Color { color_ptr[0], color_ptr[1], color_ptr[2] };
    color_ptr += 3;

unknown_color_label:
    if(err) {
        color_ptr = getUniqueColorPtr(ProvinceType::UNKNOWN);
        if(!verifyColorsRemain(ProvinceType::UNKNOWN)) {
            return Color { 0, 0, 0 }; // Last possible resort
        }

        c = Color { color_ptr[0], color_ptr[1], color_ptr[2] };
        color_ptr += 3;
    }

    return c;
}

void MapNormalizer::resetUniqueColorGenerator() {
    getUniqueColorPtr(ProvinceType::LAND) = getUniqueColorPtrStart(ProvinceType::LAND);
    getUniqueColorPtr(ProvinceType::LAKE) = getUniqueColorPtrStart(ProvinceType::LAKE);
    getUniqueColorPtr(ProvinceType::SEA) = getUniqueColorPtrStart(ProvinceType::SEA);
    getUniqueColorPtr(ProvinceType::UNKNOWN) = getUniqueColorPtrStart(ProvinceType::UNKNOWN);
}

void MapNormalizer::resetUniqueColorGenerator(ProvinceType bias) {
    getUniqueColorPtr(bias) = getUniqueColorPtrStart(bias);
}

