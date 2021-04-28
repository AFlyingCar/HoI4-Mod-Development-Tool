#ifndef MAPDRAWINGAREA_H
# define MAPDRAWINGAREA_H

# include "gtkmm/drawingarea.h"
# include "gdkmm/pixbuf.h"
# include "gdkmm/event.h"

# include "BitMap.h"

namespace MapNormalizer {
    struct Rectangle;
}

namespace MapNormalizer::GUI {
    class MapDrawingArea: public Gtk::DrawingArea {
        public:
            MapDrawingArea();
            virtual ~MapDrawingArea() = default;

            void setGraphicsDataPtr(unsigned char* const*);
            void setImagePtr(BitMap* const*);

            bool hasData() const;

            void graphicsUpdateCallback(const Rectangle&);

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

            virtual bool on_button_press_event(GdkEventButton*) override;

        private:
            unsigned char* const* m_graphics_data_ptr;
            BitMap* const* m_image_ptr;
    };
}

#endif

