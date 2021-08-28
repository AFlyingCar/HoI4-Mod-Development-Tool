
#include "GuiTests.h"

#include <fstream>
#include <string_view>

#include "gtest/gtest.h"

#include "Util.h"
#include "BitMap.h"

#include "Logger.h"
#include "ConsoleOutputFunctions.h"

#include "GuiUtils.h"

#include "TestUtils.h"

namespace MapNormalizer::UnitTests {
    class MockApplication: public Gtk::Application { };

    void GuiTests::SetUp() {
        // Set up basic outputting
        Log::Logger::registerOutputFunction(Log::outputWithFormatting);

        m_app = Glib::RefPtr<MockApplication>(new MockApplication());

        auto resource_path = getExecutablePath() / "resources.gresource.c";
        m_resources = Gio::Resource::create_from_file(resource_path.generic_string());
    }

    void GuiTests::TearDown() {
        Log::Logger::getInstance().reset();
    }

    TEST_F(GuiTests, ReadBMPFromResources) {
        // Stream #1 is the stream from the resources we want to check
        auto stream1 = m_resources->open_stream("/com/aflyingcar/MapNormalizerTools/textures/selection.bmp");

        // Stream #2 is a normal ifstream to the file on the disk
        auto real_path = getExecutablePath() / "../resources/textures/selection.bmp";
        std::ifstream stream2(real_path, std::ios::binary | std::ios::in);

        BitMap bm1, bm2;

        ASSERT_NE(readBMP(stream2, &bm2), nullptr);
        ASSERT_NE(GUI::readBMP(stream1, &bm1), nullptr);

        // Compare file_header
        {
            ASSERT_EQ(bm1.file_header.filetype, bm2.file_header.filetype);
            ASSERT_EQ(bm1.file_header.fileSize, bm2.file_header.fileSize);
            ASSERT_EQ(bm1.file_header.reserved1, bm2.file_header.reserved1);
            ASSERT_EQ(bm1.file_header.reserved2, bm2.file_header.reserved2);
            ASSERT_EQ(bm1.file_header.bitmapOffset, bm2.file_header.bitmapOffset);
        }

        // Compare info_header
        {
            ASSERT_EQ(bm1.info_header.headerSize, bm2.info_header.headerSize);
            ASSERT_EQ(bm1.info_header.width, bm2.info_header.width);
            ASSERT_EQ(bm1.info_header.height, bm2.info_header.height);
            ASSERT_EQ(bm1.info_header.bitPlanes, bm2.info_header.bitPlanes);
            ASSERT_EQ(bm1.info_header.bitsPerPixel, bm2.info_header.bitsPerPixel);
            ASSERT_EQ(bm1.info_header.compression, bm2.info_header.compression);
            ASSERT_EQ(bm1.info_header.sizeOfBitmap, bm2.info_header.sizeOfBitmap);
            ASSERT_EQ(bm1.info_header.horzResolution, bm2.info_header.horzResolution);
            ASSERT_EQ(bm1.info_header.vertResolution, bm2.info_header.vertResolution);
            ASSERT_EQ(bm1.info_header.colorsUsed, bm2.info_header.colorsUsed);
            ASSERT_EQ(bm1.info_header.colorImportant, bm2.info_header.colorImportant);
        }

        // Compare data
        {
            std::basic_string_view<unsigned char> data1(bm1.data, bm1.info_header.sizeOfBitmap);
            std::basic_string_view<unsigned char> data2(bm2.data, bm2.info_header.sizeOfBitmap);

            ASSERT_EQ(data1, data2);
        }
    }
}

