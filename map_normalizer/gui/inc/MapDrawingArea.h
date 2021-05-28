#ifndef MAPDRAWINGAREA_H
# define MAPDRAWINGAREA_H

# include <functional>

# include "gtkmm/drawingarea.h"
# include "gdkmm/pixbuf.h"
# include "gdkmm/event.h"

# include "BitMap.h"
# include "MapProject.h"

namespace MapNormalizer {
    struct Rectangle;
}

namespace MapNormalizer::GUI {
    class MapDrawingArea: public Gtk::DrawingArea {
        public:
            /**
             * @brief Holds information about the currently selected province
             */
            struct SelectionInfo {
                Project::MapProject::ProvinceDataPtr data;
                BoundingBox bounding_box;
            };

            using SelectionCallback = std::function<void(uint32_t, uint32_t)>;

            MapDrawingArea();
            virtual ~MapDrawingArea() = default;

            void setGraphicsData(const unsigned char*);
            void setImage(const BitMap*);
            void setData(const BitMap*, const unsigned char*);

            bool hasData() const;

            void graphicsUpdateCallback(const Rectangle&);

            void setOnProvinceSelectCallback(const SelectionCallback&);
            void setOnMultiProvinceSelectionCallback(const SelectionCallback&);

            void setSelection();
            void setSelection(const SelectionInfo&);

            enum class ZoomDirection {
                RESET,
                IN,
                OUT
            };

            void zoom(ZoomDirection);
            void zoom(double);

            void resetZoom();

        protected:
            virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>&) override;

            virtual bool on_button_press_event(GdkEventButton*) override;

            void rebuildImageCache();

        private:
            const unsigned char* m_graphics_data;
            const BitMap* m_image;

            /**
             * @brief A cached version of the pixbuf that gets rendered. We
             *        cache it so that we don't have to constantly rebuild it.
             */
            Glib::RefPtr<Gdk::Pixbuf> m_image_pixbuf;

            //! Called when a provice is selected
            SelectionCallback m_on_select;

            //! Called when a province is multi-selected (shift+click)
            SelectionCallback m_on_multiselect;

            //! The current selection
            std::optional<SelectionInfo> m_selection;

            //! How much should the display be scaled.
            double m_scale_factor;
    };
}

#endif

