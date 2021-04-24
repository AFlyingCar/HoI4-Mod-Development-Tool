#ifndef WINDOW_H
# define WINDOW_H

# include <vector>
# include <string>

# include <gtkmm/applicationwindow.h>
# include <gtkmm/widget.h>
# include <gtkmm/box.h>

# include "WidgetContainer.h"

namespace MapNormalizer::GUI {
    class Window: public Gtk::ApplicationWindow, public WidgetContainer {
        public:
            Window(const std::string&, Gtk::Application&);
            virtual ~Window();

            virtual bool initialize() final;

        protected:
            virtual bool initializeActions() = 0;
            virtual bool initializeWidgets() = 0;

            virtual Gtk::Orientation getDisplayOrientation() const;

            virtual void addWidgetToParent(Gtk::Widget&) override;

            Gtk::Box* getBox();

            Gtk::Application* getApplication();

        private:
            std::string m_window_name;

            Gtk::Box* m_box;

            Gtk::Application& m_application;
    };
}

#endif

