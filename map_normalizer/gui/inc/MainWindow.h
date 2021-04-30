/**
 * @file MainWindow.h
 *
 * @brief Defines the main window of the gui for the program
 */

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
    /**
     * @brief The main window
     */
    class MainWindow: public Window {
        public:
            MainWindow(Gtk::Application&);
            virtual ~MainWindow();

            virtual bool initializeActions() override;
            virtual bool initializeWidgets() override;

        protected:
            virtual Gtk::Orientation getDisplayOrientation() const override;
            virtual void addWidgetToParent(Gtk::Widget&) override;

            bool importProvinceMap(const Glib::ustring&);

            /**
             * @brief Adds a widget to this window and marks that it is now the
             *        active widget to be added to.
             */
            template<typename W, typename... Args>
            W* addActiveWidget(Args&&... args) {
                return std::get<W*>(m_active_child = addWidget<W>(std::forward<Args>(args)...));
            }

            void buildViewPane();
            Gtk::Frame* buildPropertiesPane();

            void initializeFileActions();
            void initializeEditActions();
            void initializeViewActions();
            void initializeProjectActions();

            void newProject();

            void onProjectOpened();
            void onProjectClosed();

            void saveProject();
            void saveProjectAs(const std::string& = "Save As...");

        private:
            template<typename... Args>
            using ActiveChildVariant = std::variant<std::monostate, Args*...>;

            //! The currently active widget to be added to
            ActiveChildVariant<Gtk::Box, Gtk::Frame, Gtk::ScrolledWindow> m_active_child;

            //! The currently loaded province image
            BitMap* m_image;

            //! The current graphics data to be rendered to
            unsigned char* m_graphics_data;

            //! The main pane where all child widgets will be inside
            Gtk::Paned* m_paned;

            //! The DrawingArea that the map gets rendered to.
            MapDrawingArea* m_drawing_area;
    };
}

#endif

