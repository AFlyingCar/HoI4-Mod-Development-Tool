#ifndef MAIN_WINDOW_H
# define MAIN_WINDOW_H

# include <variant>

# include "gtkmm/box.h"
# include "gtkmm/frame.h"
# include "gtkmm/paned.h"
# include "gtkmm/scrolledwindow.h"

# include "BitMap.h"

# include "Window.h"
# include "MapDrawingArea.h"

namespace MapNormalizer::GUI {
    class MainWindow: public Window {
        public:
            MainWindow(Gtk::Application&);
            virtual ~MainWindow();

            virtual bool initializeActions() override;
            virtual bool initializeWidgets() override;

        protected:
            virtual Gtk::Orientation getDisplayOrientation() const override;
            virtual void addWidgetToParent(Gtk::Widget&) override;

            bool openInputMap(const Glib::ustring&);

            template<typename W, typename... Args>
            W* addActiveWidget(Args&&... args) {
                return std::get<W*>(m_active_child = addWidget<W>(std::forward<Args>(args)...));
            }

            void buildViewPane();
            Gtk::Frame* buildPropertiesPane();

        private:
            template<typename... Args>
            using ActiveChildVariant = std::variant<std::monostate, Args*...>;

            ActiveChildVariant<Gtk::Box, Gtk::Frame, Gtk::ScrolledWindow> m_active_child;

            BitMap* m_image;
            unsigned char* m_graphics_data;

            //! The main pane where all child widgets will be inside
            Gtk::Paned* m_paned;
            MapDrawingArea* m_drawing_area;
    };
}

#endif

