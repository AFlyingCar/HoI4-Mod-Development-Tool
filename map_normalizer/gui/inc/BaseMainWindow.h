#ifndef BASEMAINWINDOW_H
# define BASEMAINWINDOW_H

# include "gtkmm/box.h"
# include "gtkmm/frame.h"
# include "gtkmm/paned.h"
# include "gtkmm/scrolledwindow.h"
# include "gtkmm/notebook.h"

# include "BaseActiveWidgetContainer.h"
# include "Window.h"

namespace MapNormalizer::GUI {
    class BaseMainWindow: public Window,
                          public virtual BaseActiveWidgetContainer<Gtk::Box,
                                                                   Gtk::Frame,
                                                                   Gtk::ScrolledWindow,
                                                                   Gtk::Notebook>
    {
        public:
            using Window::Window;

            BaseMainWindow() = delete;
            virtual ~BaseMainWindow() = default;
    };
}

#endif

