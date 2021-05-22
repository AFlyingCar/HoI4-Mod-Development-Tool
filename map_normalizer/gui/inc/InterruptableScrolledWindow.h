#ifndef INTERRUPTABLE_SCROLLED_WINDOW_H
# define INTERRUPTABLE_SCROLLED_WINDOW_H

# include "gtkmm/scrolledwindow.h"
# include "sigc++/sigc++.h"

namespace MapNormalizer::GUI {
    class InterruptableScrolledWindow: public Gtk::ScrolledWindow {
        public:
            using Gtk::ScrolledWindow::ScrolledWindow;

            using SignalOnScroll = sigc::signal<bool(GdkEventScroll*)>;

            SignalOnScroll signalOnScroll();

            virtual bool on_scroll_event(GdkEventScroll*) override;
        private:
            SignalOnScroll m_signal_on_scroll;
    };
}

#endif

