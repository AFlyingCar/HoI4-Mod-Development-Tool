#ifndef IMAPDRAWINGAREA_H
# define IMAPDRAWINGAREA_H

# include <functional>

# include "gdkmm/event.h"
# include "gtkmm/widget.h"
# include "gtkmm/container.h"

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
                [[deprecated]] Project::MapProject::ProvinceDataPtr data;
                [[deprecated]] BoundingBox bounding_box;
                ProvinceID id;
            };

            struct SelectionInfoLess {
                inline bool operator()(const SelectionInfo& s1, const SelectionInfo& s2) const { return s1.id < s2.id; };
            };

            using SelectionCallback = std::function<void(uint32_t, uint32_t)>;
            using SelectionList = std::set<SelectionInfo, SelectionInfoLess>;

            enum class ZoomDirection {
                RESET,
                IN,
                OUT
            };

            enum class ViewingMode {
                PROVINCE_VIEW,
                STATES_VIEW,
            };

            constexpr static ViewingMode DEFAULT_VIEWING_MODE = ViewingMode::PROVINCE_VIEW;

            IMapDrawingAreaBase();
            virtual ~IMapDrawingAreaBase() = default;

            void setMapData(const std::shared_ptr<const MapData>);

            ViewingMode setViewingMode(ViewingMode);

            ViewingMode getViewingMode() const;
            std::shared_ptr<const MapData> getMapData() const;

            bool hasData() const;

            virtual void graphicsUpdateCallback(const Rectangle&) = 0;

            virtual void queueDraw() = 0;
            virtual Gtk::Widget* self() = 0;
            virtual Gtk::Widget* getParent() = 0;
            virtual Glib::RefPtr<Gdk::Window> getWindow() = 0;
            virtual void hide() = 0;
            virtual void show() = 0;

            void setOnProvinceSelectCallback(const SelectionCallback&);
            void setOnMultiProvinceSelectionCallback(const SelectionCallback&);

            void setSelection();
            void setSelection(const SelectionInfo&);
            void addSelection(const SelectionInfo&);
            void removeSelection(const SelectionInfo&);

            void zoom(ZoomDirection);
            void zoom(double);

            void resetZoom();

            double getScaleFactor() const;

            // const SelectionList& getSelections() const;
            const SelectionList& getSelections() const;
            SelectionList& getSelections();

        protected:
            virtual void init() = 0;
            virtual void onZoom() = 0;

            virtual void onViewingModeChange(ViewingMode) = 0;
            virtual void onSetData(std::shared_ptr<const MapData>) = 0;
            virtual void onShow() = 0;
            virtual void onSelectionChanged(std::optional<SelectionInfo>) { };

            const SelectionCallback& getOnSelect() const;
            const SelectionCallback& getOnMultiSelect() const;

        private:
            std::shared_ptr<const MapData> m_map_data;

            //! Called when a provice is selected
            SelectionCallback m_on_select;

            //! Called when a province is multi-selected (shift+click)
            SelectionCallback m_on_multiselect;

            //! The current selection
            SelectionList m_selections;

            //! How much should the display be scaled.
            double m_scale_factor;

            //! The current viewing mode
            ViewingMode m_viewing_mode;
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

            virtual Glib::RefPtr<Gdk::Window> getWindow() override final {
                return BaseGtkWidget::get_window();
            }

            virtual void hide() override final {
                BaseGtkWidget::hide();
            }

            virtual void show() override final {
                onShow();

                BaseGtkWidget::show();
            }

            virtual Gtk::Widget* self() override final {
                return this;
            }

            // NON-FINAL OVERRIDES

            virtual void queueDraw() override {
                BaseGtkWidget::queue_draw();
            }

            virtual void graphicsUpdateCallback(const Rectangle& rectangle) override
            {
                if(rectangle.w == 0 && rectangle.h == 0) {
                    return;
                }

                BaseGtkWidget::queue_draw_area(rectangle.x, rectangle.y,
                                               rectangle.w, rectangle.h);
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

            virtual void setSizeRequest(int width = -1, int height = -1) {
                BaseGtkWidget::set_size_request(width, height);
            }
    };

    std::ostream& operator<<(std::ostream&,
                             const IMapDrawingAreaBase::ViewingMode&);
}

#endif

