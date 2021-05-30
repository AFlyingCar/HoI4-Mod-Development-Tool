
#include "gtest/gtest.h"

#include <set>
#include <thread>
#include <algorithm>
#include <atomic>
#include <cstdlib>

#include "ColorArray.h" // We use the raw arrays rather than the fancy generator
                        // functions for efficiency
#include "TestUtils.h"

struct UInt24 {
    unsigned int data: 24;
};

bool operator==(const UInt24& a, const UInt24& b) {
    return a.data == b.data;
}

bool operator<(const UInt24& a, const UInt24& b) {
    return a.data < b.data;
}

TEST(UniqueColorTests, TestColorUniqueness) {
    auto get_duplicates = [](const unsigned char* array, unsigned int size) {
        std::vector<UInt24> values(size);
        std::set<UInt24> duplicates;

        std::cout << "Sorting color array..." << std::endl;
        std::partial_sort_copy(reinterpret_cast<const UInt24*>(array),
                               reinterpret_cast<const UInt24*>(array + size),
                               values.begin(), values.end());

        std::cout << "Detecting duplicates..." << std::endl;
        for(auto i = values.begin(); i != values.end(); ++i) {
            if(auto i2 = std::next(i); i2 != values.end() && *i == *i2) {
                duplicates.insert(*i);
                i = i2;
            }
        }

        return duplicates;
    };

    std::array<std::pair<const unsigned char*, unsigned int>, 4> color_lists = {
        std::make_pair(MN_ALL_LANDS, MN_ALL_LANDS_SIZE),
        std::make_pair(MN_ALL_LAKES, MN_ALL_LAKES_SIZE),
        std::make_pair(MN_ALL_SEAS, MN_ALL_SEAS_SIZE),
        std::make_pair(MN_ALL_UNKNOWNS, MN_ALL_UNKNOWNS_SIZE)
    };

    std::vector<std::set<UInt24>> duplicate_lists;

    for(auto&& [array, size] : color_lists) {
        duplicate_lists.push_back(get_duplicates(array, size));
    }

    bool duplicate_found = std::all_of(duplicate_lists.begin(), duplicate_lists.end(), [](auto&& list) { return !list.empty(); });

    EXPECT_FALSE(duplicate_found);

    if(MapNormalizer::UnitTests::useVerboseOutput()) {
        for(auto i = 0; i < duplicate_lists.size(); ++i) {
            const auto& dup_list = duplicate_lists[i];
            auto&& [array, size] = color_lists[i];

            if(auto dup_size = dup_list.size(); dup_size > 0) {
                std::map<UInt24, std::vector<size_t>> dup_indices;

                std::cerr << "Color List " << i << " has " << dup_size << " duplicates." << std::endl;

                for(const UInt24* color = reinterpret_cast<const UInt24*>(array);
                    color < reinterpret_cast<const UInt24*>(array + size);
                    ++color)
                {
                    if(dup_list.count(*color)) {
                        // std::cerr << std::distance(reinterpret_cast<const UInt24*>(array), color) << ',';
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

