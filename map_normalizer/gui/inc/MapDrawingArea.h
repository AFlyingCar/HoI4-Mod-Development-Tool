#ifndef MAPDRAWINGAREA_H
# define MAPDRAWINGAREA_H

# include <functional>

# include "gtkmm/drawingarea.h"
# include "gdkmm/pixbuf.h"
# include "gdkmm/event.h"

# include "BitMap.h"
# include "MapProject.h"

# include "IMapDrawingArea.h"

namespace MapNormalizer {
    struct Rectangle;
}

namespace MapNormalizer::GUI {
    class MapDrawingArea: public IMapDrawingArea<Gtk::DrawingArea> {
        public:
            MapDrawingArea() = default;
            virtual ~MapDrawingArea() = default;

            virtual void graphicsUpdateCallback(const Rectangle&) override;

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

            virtual void onZoom() override;
            virtual void init() override;

            void rebuildImageCache();

        private:
            /**
             * @brief A cached version of the pixbuf that gets rendered. We
             *        cache it so that we don't have to constantly rebuild it.
             */
            Glib::RefPtr<Gdk::Pixbuf> m_image_pixbuf;
    };
}

#endif

