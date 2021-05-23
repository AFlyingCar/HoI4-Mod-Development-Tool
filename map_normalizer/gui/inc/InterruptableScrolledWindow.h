/**
 * @file InterruptableScrolledWindow.h
 *
 * @brief Defines a special ScrolledWindow whose scrolling can be interrupted
 */

#ifndef INTERRUPTABLE_SCROLLED_WINDOW_H
# define INTERRUPTABLE_SCROLLED_WINDOW_H

# include "gtkmm/scrolledwindow.h"
# include "sigc++/sigc++.h"

namespace MapNormalizer::GUI {
    /**
     * @brief A ScrolledWindow which can have its scrolling functionality get
     *        prevented.
     */
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

