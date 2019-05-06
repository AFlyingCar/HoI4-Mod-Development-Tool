#ifndef GRAPHICALDEBUGGER_H
#define GRAPHICALDEBUGGER_H

#include "BitMap.h" // BitMap
#include "Types.h" // Color

namespace MapNormalizer {
    void writeDebugColor(unsigned char*, uint32_t, uint32_t, uint32_t, Color);
    void graphicsWorker(BitMap*, unsigned char*, bool&);
}

#endif

