
#include "gtest/gtest.h"

#include <filesystem>
#include <algorithm>

#include "BitMap.h"
#include "Constants.h"
#include "StatusCodes.h"
#include "Logger.h"

#include "TestUtils.h"

TEST(BitMapTests, SimpleLoadTest) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.bmp";

    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    TEST_COUT << bmp1 << std::endl;

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, HMDT::BM_TYPE);
    ASSERT_EQ(bmp1.file_header.fileSize, 786554);
    ASSERT_EQ(bmp1.file_header.reserved1, 0);
    ASSERT_EQ(bmp1.file_header.reserved2, 0);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, 122); // fileHeader + infoHeaderV4
    // Check the info header

    // Make sure that we have the right header version (In this case, it should
    //   be the V4 Info Header)
    ASSERT_EQ(bmp1.info_header.v1.headerSize, HMDT::V4_INFO_HEADER_LENGTH);

    // If the above assertion is true, we know that we have the V4 header
    //   However, we should still be able to utilize the v1 variable to access
    //   the v1 header
    ASSERT_EQ(bmp1.info_header.v1.width, 512);
    ASSERT_EQ(bmp1.info_header.v1.height, 512);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, 1);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, 24);
    ASSERT_EQ(bmp1.info_header.v1.compression, 0);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, 786432);
    ASSERT_EQ(bmp1.info_header.v1.horzResolution, 2835);
    ASSERT_EQ(bmp1.info_header.v1.vertResolution, 2835);
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, 0);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, 0);

    // Now verify the V4 part of the header
    {
        ASSERT_EQ(bmp1.info_header.v4.redMask, 1934772034);
        ASSERT_EQ(bmp1.info_header.v4.greenMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.alphaMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.CSType, HMDT::LogicalColorSpace::CALIBRATED_RGB);

        ASSERT_EQ(bmp1.info_header.v4.redX, 0);
        ASSERT_EQ(bmp1.info_header.v4.redY, 0);
        ASSERT_EQ(bmp1.info_header.v4.redZ, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenX, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenY, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenZ, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueX, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueY, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueZ, 2);

        ASSERT_EQ(bmp1.info_header.v4.gammaRed, 0);
        ASSERT_EQ(bmp1.info_header.v4.gammaGreen, 0);
        ASSERT_EQ(bmp1.info_header.v4.gammaBlue, 0);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is no color table
    ASSERT_EQ(bmp1.color_table, nullptr);

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp1.data, nullptr);

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, Load8BPP) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "8bpp_greyscale_no_color_management.bmp";

    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, HMDT::BM_TYPE);
    ASSERT_EQ(bmp1.file_header.fileSize, 66614);
    ASSERT_EQ(bmp1.file_header.reserved1, 0);
    ASSERT_EQ(bmp1.file_header.reserved2, 0);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, 1078);
    // Check the info header

    // Make sure that we have the right header version (In this case, it should
    //   be the V4 Info Header)
    ASSERT_EQ(bmp1.info_header.v1.headerSize, HMDT::V1_INFO_HEADER_LENGTH);

    // If the above assertion is true, we know that we have the V4 header
    //   However, we should still be able to utilize the v1 variable to access
    //   the v1 header
    ASSERT_EQ(bmp1.info_header.v1.width, 256);
    ASSERT_EQ(bmp1.info_header.v1.height, 256);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, 1);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, 8);
    ASSERT_EQ(bmp1.info_header.v1.compression, 0);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, 65536);
    ASSERT_EQ(bmp1.info_header.v1.horzResolution, 11811);
    ASSERT_EQ(bmp1.info_header.v1.vertResolution, 11811);
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, 256);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, 256);

    // Do not verify the V4 part of the header as this image has no color management

    // Verify that there is a color table
    // Not really easy to test the whole color table, so as long as it is
    //   non-null then we're probably fine (can't really verify the length
    //   easily)
    ASSERT_NE(bmp1.color_table, nullptr);

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp1.data, nullptr);

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, Load8BPPWithColorManagement) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "8bpp_greyscale.bmp";

    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, HMDT::BM_TYPE);
    ASSERT_EQ(bmp1.file_header.fileSize, 66682);
    ASSERT_EQ(bmp1.file_header.reserved1, 0);
    ASSERT_EQ(bmp1.file_header.reserved2, 0);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, 1146);
    // Check the info header

    // Make sure that we have the right header version (In this case, it should
    //   be the V4 Info Header)
    ASSERT_EQ(bmp1.info_header.v1.headerSize, HMDT::V4_INFO_HEADER_LENGTH);

    // If the above assertion is true, we know that we have the V4 header
    //   However, we should still be able to utilize the v1 variable to access
    //   the v1 header
    ASSERT_EQ(bmp1.info_header.v1.width, 256);
    ASSERT_EQ(bmp1.info_header.v1.height, 256);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, 1);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, 8);
    ASSERT_EQ(bmp1.info_header.v1.compression, 0);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, 65536);
    ASSERT_EQ(bmp1.info_header.v1.horzResolution, 11811);
    ASSERT_EQ(bmp1.info_header.v1.vertResolution, 11811);
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, 256);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, 256);

    // Now verify the V4 part of the header
    {
        ASSERT_EQ(bmp1.info_header.v4.redMask, 1934772034);
        ASSERT_EQ(bmp1.info_header.v4.greenMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.alphaMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.CSType, HMDT::LogicalColorSpace::CALIBRATED_RGB);

        ASSERT_EQ(bmp1.info_header.v4.redX, 0);
        ASSERT_EQ(bmp1.info_header.v4.redY, 0);
        ASSERT_EQ(bmp1.info_header.v4.redZ, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenX, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenY, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenZ, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueX, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueY, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueZ, 2);

        ASSERT_EQ(bmp1.info_header.v4.gammaRed, 0);
        ASSERT_EQ(bmp1.info_header.v4.gammaGreen, 0);
        ASSERT_EQ(bmp1.info_header.v4.gammaBlue, 0);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is a color table
    // Not really easy to test the whole color table, so as long as it is
    //   non-null then we're probably fine (can't really verify the length
    //   easily)
    ASSERT_NE(bmp1.color_table, nullptr);

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp1.data, nullptr);

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, WriteSimpleBMP) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.bmp";
    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto bmp2_path = write_base_path / "simple_out.bmp";

    if(!std::filesystem::exists(write_base_path)) {
        TEST_COUT << "Directory " << write_base_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directory(write_base_path));
    }

    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    // Now write that BitMap back to the disk
    res = HMDT::writeBMP(bmp2_path, bmp1);
    ASSERT_SUCCEEDED(res);

    // Read the data from the BitMap back into memory
    HMDT::BitMap2 bmp2;
    res = HMDT::readBMP(bmp2_path, bmp2);
    ASSERT_SUCCEEDED(res);

    // And now verify that the file on the disk exactly matches the data we
    //   initially wrote

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, bmp2.file_header.filetype);
    ASSERT_EQ(bmp1.file_header.fileSize, bmp2.file_header.fileSize);
    ASSERT_EQ(bmp1.file_header.reserved1, bmp2.file_header.reserved1);
    ASSERT_EQ(bmp1.file_header.reserved2, bmp2.file_header.reserved2);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, bmp2.file_header.bitmapOffset);

    // Check the info header
    ASSERT_EQ(bmp1.info_header.v1.headerSize, bmp2.info_header.v1.headerSize);
    ASSERT_EQ(bmp1.info_header.v1.width, bmp2.info_header.v1.width);
    ASSERT_EQ(bmp1.info_header.v1.height, bmp2.info_header.v1.height);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, bmp2.info_header.v1.bitPlanes);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, bmp2.info_header.v1.bitsPerPixel);
    ASSERT_EQ(bmp1.info_header.v1.compression, bmp2.info_header.v1.compression);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, bmp2.info_header.v1.sizeOfBitmap);
    ASSERT_EQ(bmp1.info_header.v1.horzResolution, bmp2.info_header.v1.horzResolution);
    ASSERT_EQ(bmp1.info_header.v1.vertResolution, bmp2.info_header.v1.vertResolution);
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, bmp2.info_header.v1.colorsUsed);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, bmp2.info_header.v1.colorImportant);

    // Now verify the V4 part of the header
    {
        ASSERT_EQ(bmp1.info_header.v4.redMask, bmp2.info_header.v4.redMask);
        ASSERT_EQ(bmp1.info_header.v4.greenMask, bmp2.info_header.v4.greenMask);
        ASSERT_EQ(bmp1.info_header.v4.blueMask,bmp2.info_header.v4.blueMask);
        ASSERT_EQ(bmp1.info_header.v4.alphaMask, bmp2.info_header.v4.alphaMask);
        ASSERT_EQ(bmp1.info_header.v4.CSType, bmp2.info_header.v4.CSType);

        ASSERT_EQ(bmp1.info_header.v4.redX, bmp2.info_header.v4.redX);
        ASSERT_EQ(bmp1.info_header.v4.redY, bmp2.info_header.v4.redY);
        ASSERT_EQ(bmp1.info_header.v4.redZ, bmp2.info_header.v4.redZ);
        ASSERT_EQ(bmp1.info_header.v4.greenX, bmp2.info_header.v4.greenX);
        ASSERT_EQ(bmp1.info_header.v4.greenY, bmp2.info_header.v4.greenY);
        ASSERT_EQ(bmp1.info_header.v4.greenZ, bmp2.info_header.v4.greenZ);
        ASSERT_EQ(bmp1.info_header.v4.blueX, bmp2.info_header.v4.blueX);
        ASSERT_EQ(bmp1.info_header.v4.blueY, bmp2.info_header.v4.blueY);
        ASSERT_EQ(bmp1.info_header.v4.blueZ, bmp2.info_header.v4.blueZ);

        ASSERT_EQ(bmp1.info_header.v4.gammaRed, bmp2.info_header.v4.gammaRed);
        ASSERT_EQ(bmp1.info_header.v4.gammaGreen, bmp2.info_header.v4.gammaGreen);
        ASSERT_EQ(bmp1.info_header.v4.gammaBlue,bmp2.info_header.v4.gammaBlue);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is no color table
    ASSERT_EQ(bmp2.color_table, nullptr);

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp2.data, nullptr);

    // Now verify that the contents are equal
    ASSERT_TRUE(std::equal(bmp1.data.get(),
                           bmp1.data.get() + bmp1.info_header.v1.sizeOfBitmap,
                           bmp2.data.get()));

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, WriteSimpleBMPWithoutObject) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.bmp";
    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto bmp2_path = write_base_path / "simple_out.bmp";

    if(!std::filesystem::exists(write_base_path)) {
        TEST_COUT << "Directory " << write_base_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directory(write_base_path));
    }

    WRITE_INFO("Reading BitMap from ", bmp1_path);
    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    WRITE_INFO("Writing BitMap to ", bmp2_path);
    // Now write that BitMap back to the disk
    // Don't write using bmp1, test with the raw data + dimensions instead
    res = HMDT::writeBMP2(bmp2_path, bmp1.data.get(),
                          bmp1.info_header.v1.width, bmp1.info_header.v1.height,
                          3 /* depth */, false /* is_greyscale */,
                          HMDT::BMPHeaderToUse::V4);
    ASSERT_SUCCEEDED(res);

    WRITE_INFO("Reading BitMap from ", bmp2_path);
    // Read the data from the BitMap back into memory
    HMDT::BitMap2 bmp2;
    res = HMDT::readBMP(bmp2_path, bmp2);
    ASSERT_SUCCEEDED(res);

    // And now verify that the file on the disk exactly matches the data we
    //   initially wrote

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, bmp2.file_header.filetype);
    ASSERT_EQ(bmp1.file_header.fileSize, bmp2.file_header.fileSize);
    ASSERT_EQ(bmp1.file_header.reserved1, bmp2.file_header.reserved1);
    ASSERT_EQ(bmp1.file_header.reserved2, bmp2.file_header.reserved2);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, bmp2.file_header.bitmapOffset);

    // Check the info header
    ASSERT_EQ(bmp1.info_header.v1.headerSize, bmp2.info_header.v1.headerSize);
    ASSERT_EQ(bmp1.info_header.v1.width, bmp2.info_header.v1.width);
    ASSERT_EQ(bmp1.info_header.v1.height, bmp2.info_header.v1.height);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, bmp2.info_header.v1.bitPlanes);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, bmp2.info_header.v1.bitsPerPixel);
    ASSERT_EQ(bmp1.info_header.v1.compression, bmp2.info_header.v1.compression);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, bmp2.info_header.v1.sizeOfBitmap);
    ASSERT_EQ(bmp2.info_header.v1.horzResolution, 0); // TODO: Are these fields
    ASSERT_EQ(bmp2.info_header.v1.vertResolution, 0); //   required?
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, bmp2.info_header.v1.colorsUsed);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, bmp2.info_header.v1.colorImportant);

    // Now verify the V4 part of the header
    // This currently will _not_ match the input bitmap, since we do not
    //   build any part of the V4 header ourselves
    // TODO: If this changes in the future, make sure to update this unit test
    //   accordingly
    {

        ASSERT_EQ(bmp2.info_header.v4.redMask, HMDT::RED_MASK);
        ASSERT_EQ(bmp2.info_header.v4.greenMask, HMDT::GREEN_MASK);
        ASSERT_EQ(bmp2.info_header.v4.blueMask, HMDT::BLUE_MASK);
        ASSERT_EQ(bmp2.info_header.v4.alphaMask, 0xFF000000);
        ASSERT_EQ(bmp2.info_header.v4.CSType,
                  HMDT::LogicalColorSpace::CALIBRATED_RGB);

        ASSERT_EQ(bmp2.info_header.v4.redX, 0);
        ASSERT_EQ(bmp2.info_header.v4.redY, 0);
        ASSERT_EQ(bmp2.info_header.v4.redZ, 0);
        ASSERT_EQ(bmp2.info_header.v4.greenX, 0);
        ASSERT_EQ(bmp2.info_header.v4.greenY, 0);
        ASSERT_EQ(bmp2.info_header.v4.greenZ, 0);
        ASSERT_EQ(bmp2.info_header.v4.blueX, 0);
        ASSERT_EQ(bmp2.info_header.v4.blueY, 0);
        ASSERT_EQ(bmp2.info_header.v4.blueZ, 0);

        ASSERT_EQ(bmp2.info_header.v4.gammaRed, 0);
        ASSERT_EQ(bmp2.info_header.v4.gammaGreen, 0);
        ASSERT_EQ(bmp2.info_header.v4.gammaBlue, 0);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is no color table
    ASSERT_EQ(bmp2.color_table, nullptr);

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp2.data, nullptr);

    // Now verify that the contents are equal
    ASSERT_TRUE(std::equal(bmp1.data.get(),
                           bmp1.data.get() + bmp1.info_header.v1.sizeOfBitmap,
                           bmp2.data.get()));

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, Write8BPP) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "8bpp_greyscale_no_color_management.bmp";
    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto bmp2_path = write_base_path / "8bpp_greyscale_no_color_management_out.bmp";

    if(!std::filesystem::exists(write_base_path)) {
        TEST_COUT << "Directory " << write_base_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directory(write_base_path));
    }

    WRITE_INFO("Reading BitMap from ", bmp1_path);
    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    WRITE_DEBUG(bmp1);

    // Now write that BitMap back to the disk
    WRITE_INFO("Writing BitMap to ", bmp2_path);
    res = HMDT::writeBMP(bmp2_path, bmp1);
    ASSERT_SUCCEEDED(res);

    // Read the data from the BitMap back into memory
    WRITE_INFO("Reading BitMap from ", bmp2_path);
    HMDT::BitMap2 bmp2;
    res = HMDT::readBMP(bmp2_path, bmp2);
    ASSERT_SUCCEEDED(res);

    // And now verify that the file on the disk exactly matches the data we
    //   initially wrote

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, bmp2.file_header.filetype);
    ASSERT_EQ(bmp1.file_header.fileSize, bmp2.file_header.fileSize);
    ASSERT_EQ(bmp1.file_header.reserved1, bmp2.file_header.reserved1);
    ASSERT_EQ(bmp1.file_header.reserved2, bmp2.file_header.reserved2);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, bmp2.file_header.bitmapOffset);

    // Check the info header
    ASSERT_EQ(bmp1.info_header.v1.headerSize, bmp2.info_header.v1.headerSize);
    ASSERT_EQ(bmp1.info_header.v1.width, bmp2.info_header.v1.width);
    ASSERT_EQ(bmp1.info_header.v1.height, bmp2.info_header.v1.height);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, bmp2.info_header.v1.bitPlanes);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, bmp2.info_header.v1.bitsPerPixel);
    ASSERT_EQ(bmp1.info_header.v1.compression, bmp2.info_header.v1.compression);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, bmp2.info_header.v1.sizeOfBitmap);
    ASSERT_EQ(bmp1.info_header.v1.horzResolution, bmp2.info_header.v1.horzResolution);
    ASSERT_EQ(bmp1.info_header.v1.vertResolution, bmp2.info_header.v1.vertResolution);
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, bmp2.info_header.v1.colorsUsed);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, bmp2.info_header.v1.colorImportant);

    // Now verify the V4 part of the header
    if(bmp1.info_header.v1.headerSize == HMDT::V4_INFO_HEADER_LENGTH) {
        ASSERT_EQ(bmp1.info_header.v4.redMask, bmp2.info_header.v4.redMask);
        ASSERT_EQ(bmp1.info_header.v4.greenMask, bmp2.info_header.v4.greenMask);
        ASSERT_EQ(bmp1.info_header.v4.blueMask,bmp2.info_header.v4.blueMask);
        ASSERT_EQ(bmp1.info_header.v4.alphaMask, bmp2.info_header.v4.alphaMask);
        ASSERT_EQ(bmp1.info_header.v4.CSType, bmp2.info_header.v4.CSType);

        ASSERT_EQ(bmp1.info_header.v4.redX, bmp2.info_header.v4.redX);
        ASSERT_EQ(bmp1.info_header.v4.redY, bmp2.info_header.v4.redY);
        ASSERT_EQ(bmp1.info_header.v4.redZ, bmp2.info_header.v4.redZ);
        ASSERT_EQ(bmp1.info_header.v4.greenX, bmp2.info_header.v4.greenX);
        ASSERT_EQ(bmp1.info_header.v4.greenY, bmp2.info_header.v4.greenY);
        ASSERT_EQ(bmp1.info_header.v4.greenZ, bmp2.info_header.v4.greenZ);
        ASSERT_EQ(bmp1.info_header.v4.blueX, bmp2.info_header.v4.blueX);
        ASSERT_EQ(bmp1.info_header.v4.blueY, bmp2.info_header.v4.blueY);
        ASSERT_EQ(bmp1.info_header.v4.blueZ, bmp2.info_header.v4.blueZ);

        ASSERT_EQ(bmp1.info_header.v4.gammaRed, bmp2.info_header.v4.gammaRed);
        ASSERT_EQ(bmp1.info_header.v4.gammaGreen, bmp2.info_header.v4.gammaGreen);
        ASSERT_EQ(bmp1.info_header.v4.gammaBlue,bmp2.info_header.v4.gammaBlue);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is a color table, as it is required for <=8BPP
    ASSERT_NE(bmp1.color_table, nullptr);
    ASSERT_NE(bmp2.color_table, nullptr);

    // Now verify that the contents are equal
    auto sizeof_color_table = bmp1.info_header.v1.colorsUsed;
    int i = 0;
    ASSERT_TRUE(std::equal(bmp1.color_table.get(),
                           bmp1.color_table.get() + sizeof_color_table,
                           bmp2.color_table.get(),
                           [i](auto&& l, auto&& r) mutable {
                               if(l.rgb_quad != r.rgb_quad) {
                                   WRITE_ERROR("#", i, ": ",
                                               l.rgb_quad, " != ",  r.rgb_quad);
                               }
                               ++i;
                               return l.rgb_quad == r.rgb_quad;
                           }));

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp2.data, nullptr);

    // Now verify that the contents are equal
    i = 0;
    ASSERT_TRUE(std::equal(bmp1.data.get(),
                           bmp1.data.get() + bmp1.info_header.v1.sizeOfBitmap,
                           bmp2.data.get(),
                           [i](auto&& l, auto&& r) mutable {
                               if(l != r) {
                                   WRITE_ERROR("#", i, ": ",
                                               static_cast<uint32_t>(l),
                                               " != ",
                                               static_cast<uint32_t>(r));
                               }
                               ++i;
                               return l == r;
                           }));

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, Write8BPPWithoutObject) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "8bpp_greyscale_no_color_management.bmp";
    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto bmp2_path = write_base_path / "8bpp_greyscale_no_color_management_noobj_out.bmp";

    if(!std::filesystem::exists(write_base_path)) {
        TEST_COUT << "Directory " << write_base_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directory(write_base_path));
    }

    WRITE_INFO("Reading BitMap from ", bmp1_path);
    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    WRITE_DEBUG(bmp1);

    // Now write that BitMap back to the disk
    WRITE_INFO("Writing BitMap to ", bmp2_path);
    res = HMDT::writeBMP2(bmp2_path, bmp1.data.get(),
                          bmp1.info_header.v1.width, bmp1.info_header.v1.height,
                          1 /* depth */, true /* is_greyscale */,
                          HMDT::BMPHeaderToUse::V1);
    ASSERT_SUCCEEDED(res);

    // Read the data from the BitMap back into memory
    WRITE_INFO("Reading BitMap from ", bmp2_path);
    HMDT::BitMap2 bmp2;
    res = HMDT::readBMP(bmp2_path, bmp2);
    ASSERT_SUCCEEDED(res);

    // And now verify that the file on the disk exactly matches the data we
    //   initially wrote

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, bmp2.file_header.filetype);
    ASSERT_EQ(bmp1.file_header.fileSize, bmp2.file_header.fileSize);
    ASSERT_EQ(bmp1.file_header.reserved1, bmp2.file_header.reserved1);
    ASSERT_EQ(bmp1.file_header.reserved2, bmp2.file_header.reserved2);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, bmp2.file_header.bitmapOffset);

    // Check the info header
    ASSERT_EQ(bmp1.info_header.v1.headerSize, bmp2.info_header.v1.headerSize);
    ASSERT_EQ(bmp1.info_header.v1.width, bmp2.info_header.v1.width);
    ASSERT_EQ(bmp1.info_header.v1.height, bmp2.info_header.v1.height);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, bmp2.info_header.v1.bitPlanes);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, bmp2.info_header.v1.bitsPerPixel);
    ASSERT_EQ(bmp1.info_header.v1.compression, bmp2.info_header.v1.compression);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, bmp2.info_header.v1.sizeOfBitmap);
    ASSERT_EQ(bmp2.info_header.v1.horzResolution, 0); // TODO: Are these fields
    ASSERT_EQ(bmp2.info_header.v1.vertResolution, 0); //   required?
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, bmp2.info_header.v1.colorsUsed);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, bmp2.info_header.v1.colorImportant);

    // Now verify the V4 part of the header
    if(bmp1.info_header.v1.headerSize == HMDT::V4_INFO_HEADER_LENGTH) {
        ASSERT_EQ(bmp1.info_header.v4.redMask, bmp2.info_header.v4.redMask);
        ASSERT_EQ(bmp1.info_header.v4.greenMask, bmp2.info_header.v4.greenMask);
        ASSERT_EQ(bmp1.info_header.v4.blueMask,bmp2.info_header.v4.blueMask);
        ASSERT_EQ(bmp1.info_header.v4.alphaMask, bmp2.info_header.v4.alphaMask);
        ASSERT_EQ(bmp1.info_header.v4.CSType, bmp2.info_header.v4.CSType);

        ASSERT_EQ(bmp1.info_header.v4.redX, bmp2.info_header.v4.redX);
        ASSERT_EQ(bmp1.info_header.v4.redY, bmp2.info_header.v4.redY);
        ASSERT_EQ(bmp1.info_header.v4.redZ, bmp2.info_header.v4.redZ);
        ASSERT_EQ(bmp1.info_header.v4.greenX, bmp2.info_header.v4.greenX);
        ASSERT_EQ(bmp1.info_header.v4.greenY, bmp2.info_header.v4.greenY);
        ASSERT_EQ(bmp1.info_header.v4.greenZ, bmp2.info_header.v4.greenZ);
        ASSERT_EQ(bmp1.info_header.v4.blueX, bmp2.info_header.v4.blueX);
        ASSERT_EQ(bmp1.info_header.v4.blueY, bmp2.info_header.v4.blueY);
        ASSERT_EQ(bmp1.info_header.v4.blueZ, bmp2.info_header.v4.blueZ);

        ASSERT_EQ(bmp1.info_header.v4.gammaRed, bmp2.info_header.v4.gammaRed);
        ASSERT_EQ(bmp1.info_header.v4.gammaGreen, bmp2.info_header.v4.gammaGreen);
        ASSERT_EQ(bmp1.info_header.v4.gammaBlue,bmp2.info_header.v4.gammaBlue);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is a color table, as it is required for <=8BPP
    ASSERT_NE(bmp1.color_table, nullptr);
    ASSERT_NE(bmp2.color_table, nullptr);

    // Now verify that the contents are equal
    auto sizeof_color_table = bmp1.info_header.v1.colorsUsed;
    int i = 0;
    ASSERT_TRUE(std::equal(bmp1.color_table.get(),
                           bmp1.color_table.get() + sizeof_color_table,
                           bmp2.color_table.get(),
                           [i](auto&& l, auto&& r) mutable {
                               if(l.rgb_quad != r.rgb_quad) {
                                   WRITE_ERROR("#", i, ": ",
                                               l.rgb_quad, " != ",  r.rgb_quad);
                               }
                               ++i;
                               return l.rgb_quad == r.rgb_quad;
                           }));

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp2.data, nullptr);

    // Now verify that the contents are equal
    i = 0;
    ASSERT_TRUE(std::equal(bmp1.data.get(),
                           bmp1.data.get() + bmp1.info_header.v1.sizeOfBitmap,
                           bmp2.data.get(),
                           [i](auto&& l, auto&& r) mutable {
                               if(l != r) {
                                   WRITE_ERROR("#", i, ": ",
                                               static_cast<uint32_t>(l),
                                               " != ",
                                               static_cast<uint32_t>(r));
                               }
                               ++i;
                               return l == r;
                           }));

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, Convert24BPPTo8BPPTest) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "simple.bmp";

    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    // Load the bitmap again so we have a point of comparison
    HMDT::BitMap2 bmp2;
    res = HMDT::readBMP(bmp1_path, bmp2);
    ASSERT_SUCCEEDED(res);

    // Convert bmp1 to an 8-bit greyscale image
    res = HMDT::convertBitMapTo8BPPGreyscale(bmp1);
    ASSERT_SUCCEEDED(res);

    WRITE_DEBUG("After conversion: ", bmp1);

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, HMDT::BM_TYPE);
    ASSERT_EQ(bmp1.file_header.fileSize, 263290); // (786554 / 3) + 1024
    ASSERT_EQ(bmp1.file_header.reserved1, 0);
    ASSERT_EQ(bmp1.file_header.reserved2, 0);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, 1146); // fileHeader + infoHeaderV4 + color_table
    // Check the info header

    // Make sure that we have the right header version (In this case, it should
    //   be the V4 Info Header)
    ASSERT_EQ(bmp1.info_header.v1.headerSize, HMDT::V4_INFO_HEADER_LENGTH);

    // If the above assertion is true, we know that we have the V4 header
    //   However, we should still be able to utilize the v1 variable to access
    //   the v1 header
    ASSERT_EQ(bmp1.info_header.v1.width, 512);
    ASSERT_EQ(bmp1.info_header.v1.height, 512);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, 1);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, 8);
    ASSERT_EQ(bmp1.info_header.v1.compression, 0);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, 262144); // (786432 / 3)
    ASSERT_EQ(bmp1.info_header.v1.horzResolution, 2835);
    ASSERT_EQ(bmp1.info_header.v1.vertResolution, 2835);
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, 256);
    ASSERT_EQ(bmp1.info_header.v1.colorImportant, 256);

    // Now verify the V4 part of the header
    {
        ASSERT_EQ(bmp1.info_header.v4.redMask, 1934772034);
        ASSERT_EQ(bmp1.info_header.v4.greenMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.alphaMask, 0);
        ASSERT_EQ(bmp1.info_header.v4.CSType, HMDT::LogicalColorSpace::CALIBRATED_RGB);

        ASSERT_EQ(bmp1.info_header.v4.redX, 0);
        ASSERT_EQ(bmp1.info_header.v4.redY, 0);
        ASSERT_EQ(bmp1.info_header.v4.redZ, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenX, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenY, 0);
        ASSERT_EQ(bmp1.info_header.v4.greenZ, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueX, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueY, 0);
        ASSERT_EQ(bmp1.info_header.v4.blueZ, 2);

        ASSERT_EQ(bmp1.info_header.v4.gammaRed, 0);
        ASSERT_EQ(bmp1.info_header.v4.gammaGreen, 0);
        ASSERT_EQ(bmp1.info_header.v4.gammaBlue, 0);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is a color table
    ASSERT_NE(bmp1.color_table, nullptr);

    // Check each value in the color table
    for(uint32_t i = 0; i < bmp1.info_header.v1.colorsUsed; ++i) {
        uint8_t c = i * (0x100 / bmp1.info_header.v1.colorsUsed);

        ASSERT_EQ(bmp1.color_table[i].red, c);
        ASSERT_EQ(bmp1.color_table[i].green, c);
        ASSERT_EQ(bmp1.color_table[i].blue, c);
    }

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp1.data, nullptr);

    ::Log::Logger::getInstance().reset();
}

TEST(BitMapTests, Test8BPPCustomColorTable) {
    // We also want to see log outputs in the test output
    HMDT::UnitTests::registerTestLogOutputFunction(true, true, true, true);

    auto bmp1_path = HMDT::UnitTests::getTestProgramPath() / "bin" / "8bpp_greyscale_no_color_management.bmp";
    auto write_base_path = HMDT::UnitTests::getTestProgramPath() / "tmp";
    auto bmp2_path = write_base_path / "8bpp_custom_color_table_out.bmp";

    if(!std::filesystem::exists(write_base_path)) {
        TEST_COUT << "Directory " << write_base_path
                  << " does not exist, creating." << std::endl;
        ASSERT_TRUE(std::filesystem::create_directory(write_base_path));
    }

    WRITE_INFO("Reading BitMap from ", bmp1_path);
    HMDT::BitMap2 bmp1;
    auto res = HMDT::readBMP(bmp1_path, bmp1);
    ASSERT_SUCCEEDED(res);

    WRITE_DEBUG(bmp1);

    HMDT::ColorTable color_table {
        256 /* num_colors */,
        std::unique_ptr<HMDT::RGBQuad[]>(new HMDT::RGBQuad[256]) /* color_table */
    };

    // Build the new custom color table with 256 reds
    for(uint32_t i = 0; i < color_table.num_colors; ++i) {
        uint8_t r = static_cast<uint8_t>(i);
        color_table.color_table[i] = { { 0x00, 0x00, r, 0x00 } };
    }

    // We will lose control of the custom color table, so make a copy of it so
    //   we can later check it
    std::unique_ptr<HMDT::RGBQuad[]> backup_color_table(new HMDT::RGBQuad[256]);
    std::copy(color_table.color_table.get(),
              color_table.color_table.get() + 256,
              backup_color_table.get());

    // Now write that BitMap back to the disk, using a custom color table
    WRITE_INFO("Writing BitMap to ", bmp2_path);
    res = HMDT::writeBMP2(bmp2_path, bmp1.data.get(),
                          bmp1.info_header.v1.width, bmp1.info_header.v1.height,
                          1 /* depth */, false /* is_greyscale */,
                          HMDT::BMPHeaderToUse::V1 /* hdr_version_to_use */,
                          std::move(color_table));
    ASSERT_SUCCEEDED(res);

    // Move the backup color_table into the color_table structure
    color_table.color_table.swap(backup_color_table);

    // Read the data from the BitMap back into memory
    WRITE_INFO("Reading BitMap from ", bmp2_path);
    HMDT::BitMap2 bmp2;
    res = HMDT::readBMP(bmp2_path, bmp2);
    ASSERT_SUCCEEDED(res);

    // And now verify that the file on the disk exactly matches the data we
    //   initially wrote

    // Check the file header
    ASSERT_EQ(bmp1.file_header.filetype, bmp2.file_header.filetype);
    ASSERT_EQ(bmp1.file_header.fileSize, bmp2.file_header.fileSize);
    ASSERT_EQ(bmp1.file_header.reserved1, bmp2.file_header.reserved1);
    ASSERT_EQ(bmp1.file_header.reserved2, bmp2.file_header.reserved2);
    ASSERT_EQ(bmp1.file_header.bitmapOffset, bmp2.file_header.bitmapOffset);

    // Check the info header
    ASSERT_EQ(bmp1.info_header.v1.headerSize, bmp2.info_header.v1.headerSize);
    ASSERT_EQ(bmp1.info_header.v1.width, bmp2.info_header.v1.width);
    ASSERT_EQ(bmp1.info_header.v1.height, bmp2.info_header.v1.height);
    ASSERT_EQ(bmp1.info_header.v1.bitPlanes, bmp2.info_header.v1.bitPlanes);
    ASSERT_EQ(bmp1.info_header.v1.bitsPerPixel, bmp2.info_header.v1.bitsPerPixel);
    ASSERT_EQ(bmp1.info_header.v1.compression, bmp2.info_header.v1.compression);
    ASSERT_EQ(bmp1.info_header.v1.sizeOfBitmap, bmp2.info_header.v1.sizeOfBitmap);
    // ASSERT_EQ(bmp1.info_header.v1.horzResolution, bmp2.info_header.v1.horzResolution);
    // ASSERT_EQ(bmp1.info_header.v1.vertResolution, bmp2.info_header.v1.vertResolution);
    ASSERT_EQ(bmp1.info_header.v1.colorsUsed, bmp2.info_header.v1.colorsUsed);
    // ASSERT_EQ(bmp1.info_header.v1.colorImportant, bmp2.info_header.v1.colorImportant);

    // Now verify the V4 part of the header
    if(bmp1.info_header.v1.headerSize == HMDT::V4_INFO_HEADER_LENGTH) {
        ASSERT_EQ(bmp1.info_header.v4.redMask, bmp2.info_header.v4.redMask);
        ASSERT_EQ(bmp1.info_header.v4.greenMask, bmp2.info_header.v4.greenMask);
        ASSERT_EQ(bmp1.info_header.v4.blueMask,bmp2.info_header.v4.blueMask);
        ASSERT_EQ(bmp1.info_header.v4.alphaMask, bmp2.info_header.v4.alphaMask);
        ASSERT_EQ(bmp1.info_header.v4.CSType, bmp2.info_header.v4.CSType);

        ASSERT_EQ(bmp1.info_header.v4.redX, bmp2.info_header.v4.redX);
        ASSERT_EQ(bmp1.info_header.v4.redY, bmp2.info_header.v4.redY);
        ASSERT_EQ(bmp1.info_header.v4.redZ, bmp2.info_header.v4.redZ);
        ASSERT_EQ(bmp1.info_header.v4.greenX, bmp2.info_header.v4.greenX);
        ASSERT_EQ(bmp1.info_header.v4.greenY, bmp2.info_header.v4.greenY);
        ASSERT_EQ(bmp1.info_header.v4.greenZ, bmp2.info_header.v4.greenZ);
        ASSERT_EQ(bmp1.info_header.v4.blueX, bmp2.info_header.v4.blueX);
        ASSERT_EQ(bmp1.info_header.v4.blueY, bmp2.info_header.v4.blueY);
        ASSERT_EQ(bmp1.info_header.v4.blueZ, bmp2.info_header.v4.blueZ);

        ASSERT_EQ(bmp1.info_header.v4.gammaRed, bmp2.info_header.v4.gammaRed);
        ASSERT_EQ(bmp1.info_header.v4.gammaGreen, bmp2.info_header.v4.gammaGreen);
        ASSERT_EQ(bmp1.info_header.v4.gammaBlue,bmp2.info_header.v4.gammaBlue);
    }

    // Do not verify the V5 header part, as the size is only 108

    // Verify that there is a color table, as it is required for <=8BPP
    ASSERT_NE(bmp1.color_table, nullptr);
    ASSERT_NE(bmp2.color_table, nullptr);

    // Verify the length of the color table
    ASSERT_EQ(bmp2.info_header.v1.colorsUsed, color_table.num_colors);

    // Now verify that the contents are equal
    auto sizeof_color_table = bmp2.info_header.v1.colorsUsed;
    int i = 0;
    ASSERT_TRUE(std::equal(bmp2.color_table.get(),
                           bmp2.color_table.get() + sizeof_color_table,
                           color_table.color_table.get(),
                           [i](auto&& l, auto&& r) mutable {
                               if(l.rgb_quad != r.rgb_quad) {
                                   WRITE_ERROR("#", i, ": ",
                                               l.rgb_quad, " != ",  r.rgb_quad);
                               }
                               ++i;
                               return l.rgb_quad == r.rgb_quad;
                           }));

    // Not really easy to test the whole data-set, so as long as it is non-null
    //   then we're probably fine (can't really verify the length easily)
    ASSERT_NE(bmp2.data, nullptr);

    // Now verify that the contents are equal
    i = 0;
    ASSERT_TRUE(std::equal(bmp1.data.get(),
                           bmp1.data.get() + bmp1.info_header.v1.sizeOfBitmap,
                           bmp2.data.get(),
                           [i](auto&& l, auto&& r) mutable {
                               if(l != r) {
                                   WRITE_ERROR("#", i, ": ",
                                               static_cast<uint32_t>(l),
                                               " != ",
                                               static_cast<uint32_t>(r));
                               }
                               ++i;
                               return l == r;
                           }));

    ::Log::Logger::getInstance().reset();
}

