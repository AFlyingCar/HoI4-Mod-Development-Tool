
#include "BitMap.h"

MapNormalizer::BitMap* MapNormalizer::readBMP(const std::string& filename) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if(!file.is_open()) {
        return nullptr;
    }

    BitMap* bm = new BitMap();

    bm->filename = filename.c_str();

    // Safely read the entire header into the struct.
    if(!safeRead(&(bm->filetype), file)        ||
       !safeRead(&(bm->fileSize), file)        ||
       !safeRead(&(bm->reserved1), file)       ||
       !safeRead(&(bm->reserved2), file)       ||
       !safeRead(&(bm->bitmapOffset), file)    ||
       !safeRead(&(bm->headerSize), file)      ||
       !safeRead(&(bm->width), file)           ||
       !safeRead(&(bm->height), file)          ||
       !safeRead(&(bm->bitPlanes), file)       ||
       !safeRead(&(bm->bitsPerPixel), file)    ||
       !safeRead(&(bm->compression), file)     ||
       !safeRead(&(bm->sizeOfBitmap), file)    ||
       !safeRead(&(bm->horzResolution), file)  ||
       !safeRead(&(bm->vertResolution), file)  ||
       !safeRead(&(bm->colorsUsed), file)      ||
       !safeRead(&(bm->colorImportant), file))
    {
        // Uh oh, one of the reads failed, delete the buffer and return nullptr
        delete bm;
        return nullptr;
    }

    // Allocate space for our new image data
    bm->data = new unsigned char[bm->sizeOfBitmap];

    // Make sure we read where the file tells us the offset is, and not just
    //  assume that the data starts after the header (it doesn't always do that)
    file.seekg(bm->bitmapOffset, file.beg);

    for(size_t i = 0; i < bm->sizeOfBitmap; i += 3) {
        unsigned char bgr[3];
        if(!safeRead(bgr, 3, file)) {
            // Uh oh, couldn't read the whole thing, this means the bitmap's
            //  header is wrong, or something else happened when reading.
            delete bm->data;
            delete bm;
            return nullptr;
        }

        bm->data[i]     = bgr[2];
        bm->data[i + 1] = bgr[1];
        bm->data[i + 2] = bgr[0];
    }

    return bm;
}

