
#include "gtest/gtest.h"

#include <set>
#include <thread>
#include <algorithm>
#include <atomic>
#include <cstdlib>

#include "ColorArray.h" // We use the raw arrays rather than the fancy generator
                        // functions for efficiency in some tests
#include "UniqueColorGenerator.h"
#include "TestUtils.h"

#pragma pack(push, 1)
struct UInt24 {
    unsigned int data: 24;
};
#pragma pack(pop)

bool operator==(const UInt24& a, const UInt24& b) {
    return a.data == b.data;
}

bool operator<(const UInt24& a, const UInt24& b) {
    return a.data < b.data;
}

TEST(UniqueColorTests, TestColorUniqueness) {
    // First make sure that there is an RGB for every color value
    ASSERT_EQ(HMDT_ALL_LANDS_SIZE % 3, 0);
    ASSERT_EQ(HMDT_ALL_LAKES_SIZE % 3, 0);
    ASSERT_EQ(HMDT_ALL_SEAS_SIZE % 3, 0);
    ASSERT_EQ(HMDT_ALL_UNKNOWNS_SIZE % 3, 0);

    // We need UInt24 to be exactly 3 bytes large, as otherwise the below
    //  duplicate detection code will fail
    ASSERT_EQ(sizeof(UInt24), 3);

    auto get_duplicates = [](const unsigned char* array, unsigned int size) {
        std::vector<UInt24> values(size);
        std::set<UInt24> duplicates;

        std::copy(reinterpret_cast<const UInt24*>(array),
                  reinterpret_cast<const UInt24*>(array + size),
                  std::back_inserter(values));

        if(HMDT::UnitTests::useVerboseOutput())
            std::cerr << "Sorting color array..." << std::endl;
        std::sort(values.begin(), values.end());

        if(HMDT::UnitTests::useVerboseOutput())
            std::cerr << "Detecting duplicates..." << std::endl;
        std::set<UInt24> uniques(values.begin(), values.end());
        std::set_difference(values.begin(), values.end(),
                            uniques.begin(), uniques.end(),
                            std::inserter(duplicates, duplicates.end()));

        // Edge case: Remove this color because it shouldn't be in the list at
        //  all
        if(duplicates.count(UInt24{0})) {
            duplicates.erase(duplicates.find(UInt24{0}));
        }

        return duplicates;
    };

    std::array<std::pair<const unsigned char*, unsigned int>, 4> color_lists = {
        std::make_pair(HMDT_ALL_LANDS, HMDT_ALL_LANDS_SIZE),
        std::make_pair(HMDT_ALL_LAKES, HMDT_ALL_LAKES_SIZE),
        std::make_pair(HMDT_ALL_SEAS, HMDT_ALL_SEAS_SIZE),
        std::make_pair(HMDT_ALL_UNKNOWNS, HMDT_ALL_UNKNOWNS_SIZE)
    };

    std::vector<std::set<UInt24>> duplicate_lists;

    for(auto&& [array, size] : color_lists) {
        duplicate_lists.push_back(get_duplicates(array, size));
    }

    bool duplicate_found = std::all_of(duplicate_lists.begin(), duplicate_lists.end(), [](auto&& list) { return !list.empty(); });

    EXPECT_FALSE(duplicate_found);

    // Dump out which colors are in duplicate if $ENVVAR_VERBOSE is defined
    if(HMDT::UnitTests::useVerboseOutput()) {
        for(auto i = 0; i < duplicate_lists.size(); ++i) {
            const auto& dup_list = duplicate_lists[i];
            auto&& [array, size] = color_lists[i];

            if(auto dup_size = dup_list.size(); dup_size > 0) {
                std::map<UInt24, std::vector<size_t>> dup_indices;

                std::cerr << "Color List " << i << " has " << dup_size << " duplicates." << std::endl;

                // Find the indices for each duplicate color
                for(const UInt24* color = reinterpret_cast<const UInt24*>(array);
                    color < reinterpret_cast<const UInt24*>(array + size);
                    ++color)
                {
                    if(dup_list.count(*color)) {
                        dup_indices[*color].push_back(std::distance(reinterpret_cast<const UInt24*>(array), color));
                    }
                }

                for(auto&& [color, indices] : dup_indices) {
                    std::cerr << std::hex << color.data << ":\n\t";
                    for(auto&& index : indices) {
                        std::cerr << std::dec << index << ',';
                    }
                    std::cerr << std::endl;
                }
            }
        }
    }
}

// We want to be able to use this function despite its private nature
//  NOTE: Do not ever actually use this in practice, this is simply to make the
//   unit test easier to write and run
using UniqueColorPtr = const unsigned char*;
extern UniqueColorPtr& getUniqueColorPtr(HMDT::ProvinceType);

TEST(UniqueColorTests, TestGetUnknownsWhenOutOfColors) {
    // Make sure that we start these at the beginning
    HMDT::resetUniqueColorGenerator(HMDT::ProvinceType::UNKNOWN);

    // Force the LAND to the end
    ::getUniqueColorPtr(HMDT::ProvinceType::LAND) = HMDT_ALL_LANDS + HMDT_ALL_LANDS_SIZE;

    auto c = HMDT::generateUniqueColor(HMDT::ProvinceType::LAND);

    // Reset this again since generateUniqueColor _should_ have grabbed a value
    //  from here
    HMDT::resetUniqueColorGenerator(HMDT::ProvinceType::UNKNOWN);
    
    ASSERT_EQ(c, HMDT::generateUniqueColor(HMDT::ProvinceType::UNKNOWN));
}

