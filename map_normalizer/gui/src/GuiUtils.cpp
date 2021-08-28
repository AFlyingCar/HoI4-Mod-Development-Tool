
#include "GuiUtils.h"

#include <array>
#include <sstream>

#include "Logger.h"

namespace {
    constexpr size_t MAX_BUFFER_SIZE = 1024;
}

auto MapNormalizer::GUI::readBMP(Glib::RefPtr<Gio::InputStream> file_stream,
                                 BitMap* bm)
    -> BitMap*
{
    std::stringstream stream;
    std::array<char, MAX_BUFFER_SIZE> buffer;

    auto total_bytes_read = 0;
    for(gsize bytes_read = 0; file_stream->read_all(buffer.data(),
                                                    MAX_BUFFER_SIZE,
                                                    bytes_read);
        total_bytes_read += bytes_read)
    {
        stream.write(buffer.data(), bytes_read);

        // End the loop here if we have read all the data we need
        if(bytes_read != MAX_BUFFER_SIZE) {
            total_bytes_read += bytes_read;
            break;
        }
    }

    WRITE_DEBUG("Successfully read ", total_bytes_read, " bytes from the provided file_stream.");

    return readBMP(stream, bm);
}

