
#include "gtest/gtest.h"

#include "Util.h"

#include "TestOverrides.h"
#include "TestUtils.h"

TEST(UtilTests, FromStringTests) {
    using MapNormalizer::fromString;

    SET_PROGRAM_OPTION(quiet, true);

    // Test strings
    {
        std::string empty_string;
        std::string sstring("testString");

        ASSERT_VALID(fromString<std::string>(empty_string));
        ASSERT_VALID(fromString<std::string>(sstring));
    }

    // Test bools
    {
        std::string true_string("true");
        std::string false_string("false");

        std::string one_string("1");
        std::string zero_string("0");

        auto result = fromString<bool>(true_string);
        ASSERT_OPTIONAL(result, true);
        result = fromString<bool>(one_string);
        ASSERT_OPTIONAL(result, true);

        result = fromString<bool>(false_string);
        ASSERT_OPTIONAL(result, false);
        result = fromString<bool>(zero_string);
        ASSERT_OPTIONAL(result, false);
    }

    // Test Integers
    {
        std::string small_number("12345");
        std::string negative_number("-456789");

        auto result = fromString<int>(small_number);
        ASSERT_OPTIONAL(result, 12345);

        result = fromString<int>(negative_number);
        ASSERT_OPTIONAL(result, -456789);
    }

    // Test Floating Points
    {
        std::string pi("3.14159");

        auto result = fromString<float>(pi);
        ASSERT_OPTIONAL_FLOAT(result, 3.14159f);
    }

    // Test ProvinceType
    {
        std::string land("land");
        std::string lake("lake");
        std::string sea("sea");
        std::string unknown("unknown");
        std::string fizzbuzz("fizzbuzz");

        auto result = fromString<MapNormalizer::ProvinceType>(land);
        ASSERT_OPTIONAL(result, MapNormalizer::ProvinceType::LAND);
        result = fromString<MapNormalizer::ProvinceType>(lake);
        ASSERT_OPTIONAL(result, MapNormalizer::ProvinceType::LAKE);
        result = fromString<MapNormalizer::ProvinceType>(sea);
        ASSERT_OPTIONAL(result, MapNormalizer::ProvinceType::SEA);
        result = fromString<MapNormalizer::ProvinceType>(unknown);
        ASSERT_OPTIONAL(result, MapNormalizer::ProvinceType::UNKNOWN);
        result = fromString<MapNormalizer::ProvinceType>(fizzbuzz);
        ASSERT_OPTIONAL(result, MapNormalizer::ProvinceType::UNKNOWN);
    }
}

TEST(UtilTests, CalcDimsTests) {
    MapNormalizer::BoundingBox bb1{ { 0, 0 }, { 128, 128 } };

    ASSERT_EQ(MapNormalizer::calcDims(bb1), std::make_pair(128U, 128U));
}

TEST(UtilTests, SimpleSafeReadTests) {
    std::stringstream sstream;

    // Input data points
    uint32_t idata = 11234;
    bool bdata = true;
    float fdata = 3.14159f;
    std::string sdata = "foobar";

    // Output data points
    uint32_t read_idata;
    bool read_bdata;
    float read_fdata;
    std::shared_ptr<char[]> read_sdata(new char[sdata.size() + 1]);

    // Write data points into the stream
    MapNormalizer::writeData(sstream, idata);
    MapNormalizer::writeData(sstream, bdata);
    MapNormalizer::writeData(sstream, fdata);
    sstream.write(sdata.c_str(), sdata.size() + 1);

    // Read all of the data out of the stream
    ASSERT_TRUE(MapNormalizer::safeRead(&read_idata, sstream));
    ASSERT_TRUE(MapNormalizer::safeRead(&read_bdata, sstream));
    ASSERT_TRUE(MapNormalizer::safeRead(&read_fdata, sstream));

    // Read one more character than size() for the \0
    ASSERT_TRUE(MapNormalizer::safeRead(read_sdata.get(), sdata.size() + 1, sstream));

    // Make sure that the data got read back out of the stream correctly
    ASSERT_EQ(read_idata, idata);
    ASSERT_EQ(read_bdata, bdata);
    ASSERT_EQ(read_fdata, fdata);
    ASSERT_EQ(std::string(read_sdata.get()), sdata);
}

TEST(UtilTests, SimpleWriteDataTests) {
    std::stringstream sstream;

    uint32_t idata = 11234;
    bool bdata = true;
    float fdata = 3.14f;

    // Write the data into the stream
    MapNormalizer::writeData(sstream, idata, bdata, fdata);

    // Make sure that the datta which got written is correct
    ASSERT_EQ(idata, *reinterpret_cast<const uint32_t*>(sstream.str().c_str()));
    ASSERT_EQ(bdata, *reinterpret_cast<const bool*>(sstream.str().c_str() + sizeof(uint32_t)));
    ASSERT_FLOAT_EQ(fdata, *reinterpret_cast<const float*>(sstream.str().c_str() + sizeof(uint32_t) + sizeof(bool)));
}

TEST(UtilTests, ParseValueTests) {
    std::string data("1234 foobar true land 3.14");
    std::stringstream ss;
    ss << data;

    uint32_t idata;
    std::string sdata;
    bool bdata;
    MapNormalizer::ProvinceType pdata;
    float fdata;

    ASSERT_TRUE(MapNormalizer::parseValues(ss, idata, sdata, bdata, pdata, fdata));

    // Make sure the data got parsed out correctly
    ASSERT_EQ(idata, 1234);
    ASSERT_EQ(sdata, "foobar");
    ASSERT_EQ(bdata, true);
    ASSERT_EQ(pdata, MapNormalizer::ProvinceType::LAND);
    ASSERT_FLOAT_EQ(fdata, 3.14f);
}

TEST(UtilTests, TrimTests) {
    std::pair<std::string, std::string> ltrim_tests[] = {
        { "    ltrim   ", "ltrim   " },
        { "ltrim   ", "ltrim   " },
        { "ltrim", "ltrim" }
    };
    std::pair<std::string, std::string> rtrim_tests[] = {
        { "rtrim   ", "rtrim" },
        { "          rtrim   ", "          rtrim" }
    };
    std::pair<std::string, std::string> trim_tests[] = {
        { "   trim    ", "trim" },
        { "\ntrim", "trim" },
        { "\rtrim", "trim" },
        { "\ttrim", "trim" }
    };

    for(auto&& [test, expected] : ltrim_tests) {
        MapNormalizer::ltrim(test);
        ASSERT_EQ(test, expected);
    }

    for(auto&& [test, expected] : rtrim_tests) {
        MapNormalizer::rtrim(test);
        ASSERT_EQ(test, expected);
    }

    for(auto&& [test, expected] : trim_tests) {
        MapNormalizer::trim(test);
        ASSERT_EQ(test, expected);
    }
}

TEST(UtilTests, ClampTests) {
    ASSERT_EQ(MapNormalizer::clamp(-104, 5, -34), -34);
    ASSERT_EQ(MapNormalizer::clamp(0, 5, 20), 5);
    ASSERT_EQ(MapNormalizer::clamp(5, 5, 20), 5);
    ASSERT_EQ(MapNormalizer::clamp(11, 5, 20), 11);
    ASSERT_EQ(MapNormalizer::clamp(533, 5, 20), 20);
}

