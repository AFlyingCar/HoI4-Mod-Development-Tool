
#include "gtest/gtest.h"

#include <random>

#include "Util.h"
#include "Monad.h"

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
        std::string very_large_number(std::to_string(static_cast<uint32_t>(-1)));

        auto result = fromString<int>(small_number);
        ASSERT_OPTIONAL(result, 12345);

        result = fromString<int>(negative_number);
        ASSERT_OPTIONAL(result, -456789);

        result = fromString<uint32_t>(very_large_number);
        ASSERT_OPTIONAL(result, -1);
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

TEST(UtilTests, ParseValuesTests) {
    std::string data("1234 foobar true land 3.14");
    std::stringstream ss;
    ss << data;

    uint32_t idata;
    std::string sdata;
    bool bdata;
    MapNormalizer::ProvinceType pdata;
    float fdata;

    ASSERT_TRUE(MapNormalizer::parseValues(ss, &idata, &sdata, &bdata, &pdata, &fdata));

    // Make sure the data got parsed out correctly
    ASSERT_EQ(idata, 1234);
    ASSERT_EQ(sdata, "foobar");
    ASSERT_EQ(bdata, true);
    ASSERT_EQ(pdata, MapNormalizer::ProvinceType::LAND);
    ASSERT_FLOAT_EQ(fdata, 3.14f);
}

TEST(UtilTests, ParseValuesSkipMissingTests) {
    std::string data("1234 foobar true land 3.14");
    std::stringstream ss;
    ss << data;

    uint32_t idata;
    std::string sdata;
    bool bdata;
    char cdata = 'a';
    MapNormalizer::ProvinceType pdata;
    float fdata;
    uint32_t idata2 = 4321;

    ASSERT_TRUE(MapNormalizer::parseValuesSkipMissing(ss, &idata, &sdata, &cdata, true, &bdata, &pdata, &fdata, &idata2, true));

    // Make sure the data got parsed out correctly
    ASSERT_EQ(idata, 1234);
    ASSERT_EQ(sdata, "foobar");
    ASSERT_EQ(bdata, true);
    ASSERT_EQ(cdata, 'a');
    ASSERT_EQ(pdata, MapNormalizer::ProvinceType::LAND);
    ASSERT_FLOAT_EQ(fdata, 3.14f);
    ASSERT_EQ(idata2, 4321);
}

TEST(UtilTests, ParseValuePositionTest) {
    std::string data(" a b c d");
    std::stringstream ss;

    ss << data;

    auto start = ss.tellg();

    uint32_t idata = 1234;

    ASSERT_FALSE(MapNormalizer::parseValue(ss, idata));

    // First make sure the data wasn't touched
    ASSERT_EQ(idata, 1234);

    // Next make sure the stream is in the same position as before
    ASSERT_EQ(ss.tellg(), start);
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

TEST(UtilTests, MonadBasicTest) {
    using MapNormalizer::MonadOptional;

    std::optional<int> v = 5;

    MonadOptional<int> a(v);
    MonadOptional<int> b(std::nullopt);

    b = a;

    ASSERT_EQ(b, a);
    ASSERT_EQ(a.getWrapped(), v);
    ASSERT_TRUE(a);
    ASSERT_EQ(*a, 5);
}

TEST(UtilTests, MonadTransformTest) {
    using MapNormalizer::MonadOptional;

    struct A {
        int a;
        float b;
        char c;
    } value { 5, 3.1415f, 'z' };

    MonadOptional<A> opt_value(value);

    MonadOptional<float> v = opt_value.transform<float>([](const A& a) { return a.b; });

    ASSERT_TRUE(v);
    ASSERT_EQ(v.value(), value.b);

    MonadOptional<float> v2 = opt_value
        .transform<int>([](const A& a) { return a.a; })
        .transform<int>([](const int& a) { return a + 25; })
        .transform<float>([](const int& a) { return a * 1.254f; });

    ASSERT_TRUE(v2);
    ASSERT_EQ(v2.value(), (value.a + 25) * 1.254f);
}

TEST(UtilTests, MonadAndThenTest) {
    using MapNormalizer::MonadOptional;

    struct A {
        int a;
        float b;
        char c;
    } value { 5, 3.1415f, 'z' };

    MonadOptional<A> opt_value(value);

    MonadOptional<float> v = opt_value
        .andThen<float>([](const A& a) {
            return MonadOptional<float>(a.b);
        });

    ASSERT_TRUE(v);
    ASSERT_EQ(v.value(), value.b);

    MonadOptional<float> v2 = opt_value
        .andThen<A>([](const A& a) { return MonadOptional<A>({ a.a, 0.0f, a.c}); })
        .andThen<float>([](const A& a) { return MonadOptional<float>(static_cast<float>(a.a)); });

    ASSERT_TRUE(v2);
    ASSERT_EQ(v2.value(), static_cast<float>(value.a));
}

TEST(UtilTests, MonadOrElseTest) {
    using MapNormalizer::MonadOptional;

    struct A {
        int a;
        float b;
        char c;

        bool operator==(const A& rhs) const {
            return a == rhs.a && b == rhs.b && c == rhs.c;
        }
    } value { 5, 3.1415f, 'z' };

    MonadOptional<A> opt_value(value);

    // Verify that orElse returns opt_value when a value is held
    MonadOptional<A> opt_value2 = opt_value.orElse<void>([]() { });
    ASSERT_EQ(opt_value, opt_value2);

    // Verify that orElse returns std::nullopt if no value is held, and the
    //  function returns void
    MonadOptional<A> opt_void = MonadOptional<A>(std::nullopt)
        .orElse<void>([]() { });
    ASSERT_FALSE(opt_void);

    // Verify that we get a value from orElse
    MonadOptional<A> opt_value3 = MonadOptional<A>(std::nullopt)
        .orElse<A>([]() { return A { 25, 36.65f, 't' }; });
    ASSERT_TRUE(opt_value3);
    ASSERT_EQ(opt_value3->a, 25);
    ASSERT_EQ(opt_value3->b, 36.65f);
    ASSERT_EQ(opt_value3->c, 't');

    // Verify that values can be convertible
    MonadOptional<double> opt_value4 = MonadOptional<double>(std::nullopt)
        .orElse<float>([]() { return 3.1415f; });
    ASSERT_TRUE(opt_value4);
    ASSERT_EQ(opt_value4.value(), 3.1415f);
}

TEST(UtilTests, SimpleParallelTransformTest) {
    const uint32_t input_data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    const uint32_t expected_output_data[] = { 0, 3, 6, 9, 12, 15, 18, 21, 24, 27 };
    uint32_t output_data[10];

    MapNormalizer::parallelTransform(input_data, input_data + 10, output_data,
                                     [](uint32_t v) {
                                         return v * 3;
                                     });

    TEST_COUT << "Verifying results:" << std::endl;
    for(auto i = 0; i < 10; ++i) {
        TEST_COUT << "  ID#" << i << std::endl;
        ASSERT_EQ(output_data[i], expected_output_data[i]);
    }
}

TEST(UtilTests, LargeParallelTransformTest) {
    auto thread_count = std::thread::hardware_concurrency();
    auto num_test_values = thread_count * 10;

    std::mt19937_64 generator(static_cast<std::mt19937::result_type>(time(0)));
    std::uniform_real_distribution<double> distribution(0, num_test_values);

    // Fill our input array with random data
    std::vector<uint32_t> input_data(num_test_values);
    std::generate(input_data.begin(), input_data.end(),
                  [&distribution, &generator]() {
                      return distribution(generator);
                  });

    auto transform_func = [](uint32_t v) { return v * 3; };

    // Build the expected data array (every
    std::vector<uint32_t> expected_output_data;
    expected_output_data.reserve(num_test_values);
    std::transform(input_data.begin(), input_data.end(),
                   std::back_inserter(expected_output_data), transform_func);

    //
    std::unique_ptr<uint32_t[]> output_data(new uint32_t[num_test_values]);
    MapNormalizer::parallelTransform(input_data.begin(), input_data.end(), output_data.get(),
                                     transform_func);

    TEST_COUT << "Verifying results:" << std::endl;
    for(auto i = 0; i < num_test_values; ++i) {
        TEST_COUT << "  ID#" << i << std::endl;
        ASSERT_EQ(output_data[i], expected_output_data[i]);
    }
}
