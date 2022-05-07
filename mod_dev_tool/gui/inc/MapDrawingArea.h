#ifndef MAPDRAWINGAREA_H
# define MAPDRAWINGAREA_H

# include <functional>

# include "gtkmm/drawingarea.h"
# include "gdkmm/pixbuf.h"
# include "gdkmm/event.h"

# include "BitMap.h"
# include "MapProject.h"

# include "IMapDrawingArea.h"

namespace HMDT {
    struct Rectangle;
}

namespace HMDT::GUI {
    class MapDrawingArea: public IMapDrawingArea<Gtk::DrawingArea> {
        public:
            MapDrawingArea() = default;
            virtual ~MapDrawingArea() = default;

            void rebuildImageCache();

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

            virtual void onZoom() override;
            virtual void init() override;

            virtual void onShow() override { };
            virtual void onViewingModeChange(ViewingMode) override { };
            virtual void onSetData(std::shared_ptr<const MapData>) override
            { };

        private:
            /**
             * @brief A cached version of the pixbuf that gets rendered. We
             *        cache it so that we don't have to constantly rebuild it.
             */
            Glib::RefPtr<Gdk::Pixbuf> m_image_pixbuf;
    };
}

#endif

