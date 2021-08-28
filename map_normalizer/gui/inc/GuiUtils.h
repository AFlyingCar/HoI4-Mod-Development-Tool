
#ifndef GUI_UTILS_H
# define GUI_UTILS_H

# include "BitMap.h"

# include "giomm/inputstream.h"

namespace MapNormalizer::GUI {
    BitMap* readBMP(Glib::RefPtr<Gio::InputStream>, BitMap*);
}

#endif

