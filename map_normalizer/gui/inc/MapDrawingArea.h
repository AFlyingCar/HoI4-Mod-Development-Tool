#ifndef MAPDRAWINGAREA_H
# define MAPDRAWINGAREA_H

# include "gtkmm/drawingarea.h"
# include "gdkmm/pixbuf.h"

# include "BitMap.h"

namespace MapNormalizer::GUI {
    class MapDrawingArea: public Gtk::DrawingArea {
        public:
            MapDrawingArea();
            virtual ~MapDrawingArea() = default;

            void setGraphicsDataPtr(unsigned char* const*);
            void setImagePtr(BitMap* const*);

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

        private:
            unsigned char* const* m_graphics_data_ptr;
            BitMap* const* m_image_ptr;
    };
}

#endif

