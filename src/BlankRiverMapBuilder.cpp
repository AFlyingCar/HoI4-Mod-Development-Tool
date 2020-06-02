
#include "BlankRiverMapBuilder.h"

#include "ProvinceMapBuilder.h"
#include "Util.h"

auto MapNormalizer::buildBlankRiverMap(const ProvinceList& provinces,
                                       const BitMap* input,
                                       const PolygonList& shapes) -> BitMap*
{
    BitMap* blank_river = new BitMap();

    // Make sure the new BitMap contains all the same image information as the
    //  original
    *blank_river = *input;

    // blank_river should have a new data array though
    blank_river->data = new unsigned char[input->info_header.width * 3 * input->info_header.height];
    for(auto&& province : provinces) {
        auto& shape = shapes[province.id];
        bool isLand = getProvinceType(shape.color) == ProvinceType::LAND;

        for(auto&& pix : shape.pixels) {
            uint32_t index = xyToIndex(blank_river, pix.point.x, pix.point.y);

            // Land is 0xFFFFFF
            // Everything else is 0x7A7A7A
            if(isLand) {
                blank_river->data[index] = 0xFF;
                blank_river->data[index + 1] = 0xFF;
                blank_river->data[index + 2] = 0xFF;
            } else {
                blank_river->data[index] = 0x7A;
                blank_river->data[index + 1] = 0x7A;
                blank_river->data[index + 2] = 0x7A;
            }
        }
    }

    return blank_river;
}
