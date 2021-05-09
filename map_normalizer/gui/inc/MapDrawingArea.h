#ifndef MAPDRAWINGAREA_H
# define MAPDRAWINGAREA_H

# include <functional>

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
            using SelectionCallback = std::function<void(uint32_t, uint32_t)>;

            MapDrawingArea();
            virtual ~MapDrawingArea() = default;

            void setGraphicsData(const unsigned char*);
            void setImage(const BitMap*);

            bool hasData() const;

            void graphicsUpdateCallback(const Rectangle&);

            void setOnProvinceSelectCallback(const SelectionCallback&);
            void setOnMultiProvinceSelectionCallback(const SelectionCallback&);

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

            virtual bool on_button_press_event(GdkEventButton*) override;

        private:
            const unsigned char* m_graphics_data;
            const BitMap* m_image;

            SelectionCallback m_on_select;
            SelectionCallback m_on_multiselect;
    };
}

#endif

