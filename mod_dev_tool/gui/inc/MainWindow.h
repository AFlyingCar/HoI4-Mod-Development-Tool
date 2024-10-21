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
# include "MainWindowFileTreePart.h"

# include "ConfigEditorWindow.h"
# include "LogViewerWindow.h"
# include "Toolbar.h"
# include "AddFileWindow.h"

namespace HMDT::GUI {
    /**
     * @brief The main window
     */
    class MainWindow: public virtual BaseMainWindow,
                      public MainWindowDrawingAreaPart,
                      public MainWindowPropertiesPanePart,
                      public MainWindowFileTreePart
    {
        public:
            //! The default position of the file tree pane
            static constexpr std::int32_t DEFAULT_FILE_TREE_POSITION = 330;

            MainWindow(Gtk::Application&);
            virtual ~MainWindow();

            virtual bool initializeActions() override;
            virtual bool initializeWidgets() override;
            virtual bool initializeFinal() override;

            OptionalReference<LogViewerWindow> getLogViewerWindow();

            template<typename T>
            T* thisAs() const noexcept {
                static_assert(std::is_base_of_v<T, MainWindow>,
                              "Can only get MainWindow as one if its base "
                              "classes.");

                return dynamic_cast<T*>(this);
            }

        protected:
            virtual void updatePart(const PartType&, const std::any&) noexcept override;

            virtual Gtk::Orientation getDisplayOrientation() const override;
            virtual void addWidgetToParent(Gtk::Widget&) override;

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

            //! Sub-pane for the left-hand of the screen, allowing us to have 3 panes
            Gtk::Paned* m_left_pane;

            //! The window for viewing the logs
            std::unique_ptr<LogViewerWindow> m_log_viewer_window;

            //! The window for editing the config
            std::unique_ptr<ConfigEditorWindow> m_config_editor_window;

            //! The window for adding files into the current project
            std::unique_ptr<AddFileWindow> m_add_file_window;
    };
}

#endif

