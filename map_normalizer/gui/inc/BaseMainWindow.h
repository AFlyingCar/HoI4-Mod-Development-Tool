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

            // NOTE: This is left undefined on purpose. Constructing a
            //   BaseMainWindow _must_ be done through the base Window
            //   non-default constructor. Do not attempt to define this.
            BaseMainWindow();
            virtual ~BaseMainWindow() = default;
    };
}

#endif

