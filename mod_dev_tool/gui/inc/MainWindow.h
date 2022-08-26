/**
 * @file MainWindow.h
 *
 * @brief Defines the main window of the gui for the program
 */

#ifndef MAIN_WINDOW_H
# define MAIN_WINDOW_H

# include <functional>
# include <variant>

# include "BitMap.h"
# include "Types.h"

# include "BaseMainWindow.h"
# include "MainWindowDrawingAreaPart.h"
# include "MainWindowPropertiesPanePart.h"

# include "ConfigEditorWindow.h"
# include "LogViewerWindow.h"
# include "Toolbar.h"

namespace HMDT::GUI {
    /**
     * @brief The main window
     */
    class MainWindow: public virtual BaseMainWindow,
                      public MainWindowDrawingAreaPart,
                      public MainWindowPropertiesPanePart
    {
        public:
            MainWindow(Gtk::Application&);
            virtual ~MainWindow();

            virtual bool initializeActions() override;
            virtual bool initializeWidgets() override;
            virtual bool initializeFinal() override;

            OptionalReference<LogViewerWindow> getLogViewerWindow();

        protected:
            virtual Gtk::Orientation getDisplayOrientation() const override;
            virtual void addWidgetToParent(Gtk::Widget&) override;

            bool importProvinceMap(const Glib::ustring&);

            void buildToolbar();
            void buildViewPane();

            void initializeCallbacks();

            void initializeFileActions();
            void initializeEditActions();
            void initializeViewActions();
            void initializeProjectActions();
            void initializeHelpActions();

            void newProject();
            void openProject();

            void onProjectOpened();
            void onProjectClosed();

            void saveProject();
            void saveProjectAs(const std::string& = "Save As...");

            void exportProject();
            void exportProjectAs(const std::string& = "Export To...");

        private:
            //! The toolbar of the application
            Toolbar* m_toolbar;

            //! The main pane where all child widgets will be inside
            Gtk::Paned* m_paned;

            //! The window for viewing the logs
            std::unique_ptr<LogViewerWindow> m_log_viewer_window;

            //! The window for editing the config
            std::unique_ptr<ConfigEditorWindow> m_config_editor_window;
    };
}

#endif

