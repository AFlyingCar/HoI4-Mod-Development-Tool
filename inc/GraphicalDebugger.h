/**
 * @file GraphicalDebugger.h
 *
 * @brief Defines various functions for the graphical debugger.
 */

#ifndef GRAPHICALDEBUGGER_H
# define GRAPHICALDEBUGGER_H

# include "BitMap.h"
# include "Types.h"

namespace MapNormalizer {
    void writeDebugColor(unsigned char*, uint32_t, uint32_t, uint32_t, Color);
    void graphicsWorker(BitMap*, unsigned char*, bool&);

    void checkForPause();
}

#endif

