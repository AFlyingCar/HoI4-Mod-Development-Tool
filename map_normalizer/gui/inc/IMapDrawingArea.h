#ifndef IMAPDRAWINGAREA_H
# define IMAPDRAWINGAREA_H

# include <functional>

# include "gdkmm/event.h"
# include "gtkmm/widget.h"

# include "BitMap.h"
# include "MapProject.h"

namespace MapNormalizer {
    struct Rectangle;
}

namespace MapNormalizer::GUI {
    /**
     * @brief Stores all non-gtk components
     */
    class IMapDrawingAreaBase {
        public:
            /**
             * @brief Holds information about the currently selected province
             */
            struct SelectionInfo {
                Project::MapProject::ProvinceDataPtr data;
                BoundingBox bounding_box;
            };

            using SelectionCallback = std::function<void(uint32_t, uint32_t)>;

            enum class ZoomDirection {
                RESET,
                IN,
                OUT
            };

            IMapDrawingAreaBase();
            virtual ~IMapDrawingAreaBase() = default;

            void setGraphicsData(const unsigned char*);
            void setImage(const BitMap*);
            void setData(const BitMap*, const unsigned char*);

            const unsigned char* getGraphicsData() const;
            const BitMap* getImage() const;

            bool hasData() const;

            virtual void graphicsUpdateCallback(const Rectangle&) = 0;

            void setOnProvinceSelectCallback(const SelectionCallback&);
            void setOnMultiProvinceSelectionCallback(const SelectionCallback&);

            void setSelection();
            void setSelection(const SelectionInfo&);

            void zoom(ZoomDirection);
            void zoom(double);

            void resetZoom();

            double getScaleFactor() const;

        protected:
            virtual void init() = 0;
            virtual void onZoom() = 0;

            const SelectionCallback& getOnSelect() const;
            const SelectionCallback& getOnMultiSelect() const;

            const std::optional<SelectionInfo>& getSelection() const;
            std::optional<SelectionInfo>& getSelection();

            virtual Gtk::Widget* getParent() = 0;

        private:
            const unsigned char* m_graphics_data;
            const BitMap* m_image;

            //! Called when a provice is selected
            SelectionCallback m_on_select;

            //! Called when a province is multi-selected (shift+click)
            SelectionCallback m_on_multiselect;

            //! The current selection
            std::optional<SelectionInfo> m_selection;

            //! How much should the display be scaled.
            double m_scale_factor;
    };

    template<typename BaseGtkWidget>
    class IMapDrawingArea: public IMapDrawingAreaBase, public BaseGtkWidget {
        public:
            IMapDrawingArea() {
                // Mark that we want to receive button presses
                BaseGtkWidget::add_events(Gdk::BUTTON_PRESS_MASK);
            }

            virtual ~IMapDrawingArea() = default;

            virtual Gtk::Widget* getParent() override final {
                return BaseGtkWidget::get_parent();
            }

        protected:
            virtual bool on_button_press_event(GdkEventButton* event) override {
                if(!hasData()) {
                    return true;
                }

                // Is it a left-click?
                if(event->type == GDK_BUTTON_PRESS && event->button == 1) {
                    // Note that x and y will be the values after scaling. If we want the
                    //  true coordinates, we have to invert the scaling
                    auto x = event->x * (1 / getScaleFactor());
                    auto y = event->y * (1 / getScaleFactor());

                    if(event->state & GDK_SHIFT_MASK) {
                        getOnMultiSelect()(x, y);
                    } else {
                        getOnSelect()(x, y);
                    }
                }

                return true;
            }
    };
}

#endif

